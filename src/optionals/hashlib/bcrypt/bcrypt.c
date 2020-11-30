/*	$OpenBSD: bcrypt.c,v 1.58 2020/07/06 13:33:05 pirofti Exp $	*/

/*
 * Copyright (c) 2014 Ted Unangst <tedu@openbsd.org>
 * Copyright (c) 1997 Niels Provos <provos@umich.edu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* This password hashing algorithm was designed by David Mazieres
 * <dm@lcs.mit.edu> and works as follows:
 *
 * 1. state := InitState ()
 * 2. state := ExpandKey (state, salt, password)
 * 3. REPEAT rounds:
 *      state := ExpandKey (state, 0, password)
 *	state := ExpandKey (state, 0, salt)
 * 4. ctext := "OrpheanBeholderScryDoubt"
 * 5. REPEAT 64:
 * 	ctext := Encrypt_ECB (state, ctext);
 * 6. RETURN Concatenate (salt, ctext);
 *
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <bcrypt.h>
#endif

#ifdef __linux__
    #if __GLIBC__ > 2 || __GLIBC_MINOR__ > 24
        #include <sys/random.h>
    #elif defined(__GLIBC__)
        #include <unistd.h>
        #include <sys/syscall.h>
        #include <errno.h>
    #else
        #include <unistd.h>
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>
    #endif
#endif

#include "blf.h"

/* This implementation is adaptable to current computing power.
 * You can have up to 2^31 rounds which should be enough for some
 * time to come.
 */

#define BCRYPT_VERSION '2'
#define BCRYPT_MAXSALT 16	/* Precomputation is just so nice */
#define BCRYPT_WORDS 6		/* Ciphertext words */
#define BCRYPT_MINLOGROUNDS 4	/* we have log2(rounds) in salt */

#define	BCRYPT_SALTSPACE	(7 + (BCRYPT_MAXSALT * 4 + 2) / 3 + 1)
#define	BCRYPT_HASHSPACE	61

char   *bcrypt_gensalt(u_int8_t);

static int encode_base64(char *, const u_int8_t *, size_t);
static int decode_base64(u_int8_t *, size_t, const char *);
/*
 * Generates a salt for this version of crypt.
 */
static int
bcrypt_initsalt(int log_rounds, uint8_t *salt, size_t saltbuflen)
{
    uint8_t csalt[BCRYPT_MAXSALT];

    if (saltbuflen < BCRYPT_SALTSPACE) {
        errno = EINVAL;
        return -1;
    }

#ifdef _WIN32
    NTSTATUS Status = BCryptGenRandom(NULL, csalt, sizeof(csalt), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    if (Status < 0) {
        return -1;
    }
#elif defined __linux__
    #if __GLIBC__ > 2 || __GLIBC_MINOR__ > 24
        if (getentropy(csalt, sizeof(csalt)) != 0) {
            return -1;
        }
    #elif defined(__GLIBC__)
        if (syscall(SYS_getrandom, csalt, sizeof(csalt), 0) == -1) {
            return -1;
        }
    #else
        int randomData = open("/dev/urandom", O_RDONLY);
        if (randomData > 0) {
            ssize_t result = read(randomData, csalt, sizeof(csalt));
            if (result < 0) return -1;
        } else {
            return -1;
        }
    #endif
#else
    arc4random_buf(csalt, sizeof(csalt));
#endif // _WIN32

    if (log_rounds < 4)
        log_rounds = 4;
    else if (log_rounds > 31)
        log_rounds = 31;

    snprintf((char*)salt, saltbuflen, "$2b$%2.2u$", log_rounds);
    encode_base64((char*)salt + 7, csalt, sizeof(csalt));

    return 0;
}

/*
 * the core bcrypt function
 */
static int
bcrypt_hashpass(const char *key, const char *salt, char *encrypted,
                size_t encryptedlen)
{
    blf_ctx state;
    u_int32_t rounds, i, k;
    u_int16_t j;
    size_t key_len;
    u_int8_t salt_len, logr, minor;
    u_int8_t ciphertext[4 * BCRYPT_WORDS] = "OrpheanBeholderScryDoubt";
    u_int8_t csalt[BCRYPT_MAXSALT];
    u_int32_t cdata[BCRYPT_WORDS];

    if (encryptedlen < BCRYPT_HASHSPACE)
        goto inval;

    /* Check and discard "$" identifier */
    if (salt[0] != '$')
        goto inval;
    salt += 1;

    if (salt[0] != BCRYPT_VERSION)
        goto inval;

    /* Check for minor versions */
    switch ((minor = salt[1])) {
        case 'a':
            key_len = (u_int8_t)(strlen(key) + 1);
            break;
        case 'b':
            /* strlen() returns a size_t, but the function calls
             * below result in implicit casts to a narrower integer
             * type, so cap key_len at the actual maximum supported
             * length here to avoid integer wraparound */
            key_len = strlen(key);
            if (key_len > 72)
                key_len = 72;
            key_len++; /* include the NUL */
            break;
        default:
            goto inval;
    }
    if (salt[2] != '$')
        goto inval;
    /* Discard version + "$" identifier */
    salt += 3;

    /* Check and parse num rounds */
    if (!isdigit((unsigned char)salt[0]) ||
        !isdigit((unsigned char)salt[1]) || salt[2] != '$')
        goto inval;
    logr = (salt[1] - '0') + ((salt[0] - '0') * 10);
    if (logr < BCRYPT_MINLOGROUNDS || logr > 31)
        goto inval;
    /* Computer power doesn't increase linearly, 2^x should be fine */
    rounds = 1U << logr;

    /* Discard num rounds + "$" identifier */
    salt += 3;

    if (strlen(salt) * 3 / 4 < BCRYPT_MAXSALT)
        goto inval;

    /* We dont want the base64 salt but the raw data */
    if (decode_base64(csalt, BCRYPT_MAXSALT, salt))
        goto inval;
    salt_len = BCRYPT_MAXSALT;

    /* Setting up S-Boxes and Subkeys */
    Blowfish_initstate(&state);
    Blowfish_expandstate(&state, csalt, salt_len,
                         (u_int8_t *) key, key_len);
    for (k = 0; k < rounds; k++) {
        Blowfish_expand0state(&state, (u_int8_t *) key, key_len);
        Blowfish_expand0state(&state, csalt, salt_len);
    }

    /* This can be precomputed later */
    j = 0;
    for (i = 0; i < BCRYPT_WORDS; i++)
        cdata[i] = Blowfish_stream2word(ciphertext, 4 * BCRYPT_WORDS, &j);

    /* Now do the encryption */
    for (k = 0; k < 64; k++)
        blf_enc(&state, cdata, BCRYPT_WORDS / 2);

    for (i = 0; i < BCRYPT_WORDS; i++) {
        ciphertext[4 * i + 3] = cdata[i] & 0xff;
        cdata[i] = cdata[i] >> 8;
        ciphertext[4 * i + 2] = cdata[i] & 0xff;
        cdata[i] = cdata[i] >> 8;
        ciphertext[4 * i + 1] = cdata[i] & 0xff;
        cdata[i] = cdata[i] >> 8;
        ciphertext[4 * i + 0] = cdata[i] & 0xff;
    }


    snprintf(encrypted, 8, "$2%c$%2.2u$", minor, logr);
    encode_base64(encrypted + 7, csalt, BCRYPT_MAXSALT);
    encode_base64(encrypted + 7 + 22, ciphertext, 4 * BCRYPT_WORDS - 1);
    explicit_bzero(&state, sizeof(state));
    explicit_bzero(ciphertext, sizeof(ciphertext));
    explicit_bzero(csalt, sizeof(csalt));
    explicit_bzero(cdata, sizeof(cdata));
    return 0;

    inval:
    errno = EINVAL;
    return -1;
}

/*
 * user friendly functions
 */
int
bcrypt_newhash(const char *pass, int log_rounds, char *hash, size_t hashlen)
{
    char salt[BCRYPT_SALTSPACE];

    if (bcrypt_initsalt(log_rounds, (u_int8_t*)salt, sizeof(salt)) != 0)
        return -1;

    if (bcrypt_hashpass(pass, salt, hash, hashlen) != 0)
        return -1;

    explicit_bzero(salt, sizeof(salt));
    return 0;
}
DEF_WEAK(bcrypt_newhash);

int
bcrypt_checkpass(const char *pass, const char *goodhash)
{
    char hash[BCRYPT_HASHSPACE];

    if (bcrypt_hashpass(pass, goodhash, hash, sizeof(hash)) != 0)
        return -1;
    if (strlen(hash) != strlen(goodhash) ||
        timingsafe_bcmp(hash, goodhash, strlen(goodhash)) != 0) {
        errno = EACCES;
        return -1;
    }

    explicit_bzero(hash, sizeof(hash));
    return 0;
}
DEF_WEAK(bcrypt_checkpass);

/*
 * internal utilities
 */
static const u_int8_t Base64Code[] =
        "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static const u_int8_t index_64[128] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 0, 1, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63, 255, 255,
        255, 255, 255, 255, 255, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
        255, 255, 255, 255, 255, 255, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 255, 255, 255, 255, 255
};
#define CHAR64(c)  ( (c) > 127 ? 255 : index_64[(c)])

/*
 * read buflen (after decoding) bytes of data from b64data
 */
static int
decode_base64(u_int8_t *buffer, size_t len, const char *b64data)
{
    u_int8_t *bp = buffer;
    const u_int8_t *p = (u_int8_t*)b64data;
    u_int8_t c1, c2, c3, c4;

    while (bp < buffer + len) {
        c1 = CHAR64(*p);
        /* Invalid data */
        if (c1 == 255)
            return -1;

        c2 = CHAR64(*(p + 1));
        if (c2 == 255)
            return -1;

        *bp++ = (c1 << 2) | ((c2 & 0x30) >> 4);
        if (bp >= buffer + len)
            break;

        c3 = CHAR64(*(p + 2));
        if (c3 == 255)
            return -1;

        *bp++ = ((c2 & 0x0f) << 4) | ((c3 & 0x3c) >> 2);
        if (bp >= buffer + len)
            break;

        c4 = CHAR64(*(p + 3));
        if (c4 == 255)
            return -1;
        *bp++ = ((c3 & 0x03) << 6) | c4;

        p += 4;
    }
    return 0;
}

/*
 * Turn len bytes of data into base64 encoded data.
 * This works without = padding.
 */
static int
encode_base64(char *b64buffer, const u_int8_t *data, size_t len)
{
    u_int8_t *bp = (u_int8_t*)b64buffer;
    const u_int8_t *p = data;
    u_int8_t c1, c2;

    while (p < data + len) {
        c1 = *p++;
        *bp++ = Base64Code[(c1 >> 2)];
        c1 = (c1 & 0x03) << 4;
        if (p >= data + len) {
            *bp++ = Base64Code[c1];
            break;
        }
        c2 = *p++;
        c1 |= (c2 >> 4) & 0x0f;
        *bp++ = Base64Code[c1];
        c1 = (c2 & 0x0f) << 2;
        if (p >= data + len) {
            *bp++ = Base64Code[c1];
            break;
        }
        c2 = *p++;
        c1 |= (c2 >> 6) & 0x03;
        *bp++ = Base64Code[c1];
        *bp++ = Base64Code[c2 & 0x3f];
    }
    *bp = '\0';
    return 0;
}

/*
 * classic interface
 */
char *
bcrypt_gensalt(u_int8_t log_rounds)
{
    static char    gsalt[BCRYPT_SALTSPACE];

    bcrypt_initsalt(log_rounds, (u_int8_t*)gsalt, sizeof(gsalt));

    return gsalt;
}

char *
bcrypt_pass(const char *pass, const char *salt)
{
    static char    gencrypted[BCRYPT_HASHSPACE];

    if (bcrypt_hashpass(pass, salt, gencrypted, sizeof(gencrypted)) != 0)
        return NULL;

    return gencrypted;
}
DEF_WEAK(bcrypt);