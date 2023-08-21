#include "c.h"

#ifdef _WIN32
#define strerror_r(ERRNO, BUF, LEN) strerror_s(BUF, LEN, ERRNO)
#endif

void getStrerror(char *buf, int error) {
#ifdef POSIX_STRERROR
    int intval = strerror_r(error, buf, MAX_ERROR_LEN);

    if (intval == 0) {
        return;
    }

    /* it is safe to assume that we do not reach here */
    memcpy(buf, "Buffer is too small", 19);
    buf[19] = '\0';
#else
    strerror_r(error, buf, MAX_ERROR_LEN);
#endif
}

Value createCModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "C", 1);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define C properties
     */
    defineNativeProperty(vm, &module->values, "EPERM",  NUMBER_VAL(EPERM));
    defineNativeProperty(vm, &module->values, "ENOENT", NUMBER_VAL(ENOENT));
    defineNativeProperty(vm, &module->values, "ESRCH",  NUMBER_VAL(ESRCH));
    defineNativeProperty(vm, &module->values, "EINTR",  NUMBER_VAL(EINTR));
    defineNativeProperty(vm, &module->values, "EIO",    NUMBER_VAL(EIO));
    defineNativeProperty(vm, &module->values, "ENXIO",  NUMBER_VAL(ENXIO));
    defineNativeProperty(vm, &module->values, "E2BIG",  NUMBER_VAL(E2BIG));
    defineNativeProperty(vm, &module->values, "ENOEXEC",NUMBER_VAL(ENOEXEC));
    defineNativeProperty(vm, &module->values, "EAGAIN", NUMBER_VAL(EAGAIN));
    defineNativeProperty(vm, &module->values, "ENOMEM", NUMBER_VAL(ENOMEM));
    defineNativeProperty(vm, &module->values, "EACCES", NUMBER_VAL(EACCES));
    defineNativeProperty(vm, &module->values, "EFAULT", NUMBER_VAL(EFAULT));
#ifdef ENOTBLK
    defineNativeProperty(vm, &module->values, "ENOTBLK", NUMBER_VAL(ENOTBLK));
#endif
    defineNativeProperty(vm, &module->values, "EBUSY",  NUMBER_VAL(EBUSY));
    defineNativeProperty(vm, &module->values, "EEXIST", NUMBER_VAL(EEXIST));
    defineNativeProperty(vm, &module->values, "EXDEV",  NUMBER_VAL(EXDEV));
    defineNativeProperty(vm, &module->values, "ENODEV", NUMBER_VAL(ENODEV));
    defineNativeProperty(vm, &module->values, "ENOTDIR",NUMBER_VAL(ENOTDIR));
    defineNativeProperty(vm, &module->values, "EISDIR", NUMBER_VAL(EISDIR));
    defineNativeProperty(vm, &module->values, "EINVAL", NUMBER_VAL(EINVAL));
    defineNativeProperty(vm, &module->values, "ENFILE", NUMBER_VAL(ENFILE));
    defineNativeProperty(vm, &module->values, "EMFILE", NUMBER_VAL(EMFILE));
    defineNativeProperty(vm, &module->values, "ENOTTY", NUMBER_VAL(ENOTTY));
    defineNativeProperty(vm, &module->values, "ETXTBSY",NUMBER_VAL(ETXTBSY));
    defineNativeProperty(vm, &module->values, "EFBIG",  NUMBER_VAL(EFBIG));
    defineNativeProperty(vm, &module->values, "ENOSPC", NUMBER_VAL(ENOSPC));
    defineNativeProperty(vm, &module->values, "ESPIPE", NUMBER_VAL(ESPIPE));
    defineNativeProperty(vm, &module->values, "EROFS",  NUMBER_VAL(EROFS));
    defineNativeProperty(vm, &module->values, "EMLINK", NUMBER_VAL(EMLINK));
    defineNativeProperty(vm, &module->values, "EPIPE",  NUMBER_VAL(EPIPE));
    defineNativeProperty(vm, &module->values, "EDOM",   NUMBER_VAL(EDOM));
    defineNativeProperty(vm, &module->values, "ERANGE", NUMBER_VAL(ERANGE));
    defineNativeProperty(vm, &module->values, "EDEADLK",NUMBER_VAL(EDEADLK));
    defineNativeProperty(vm, &module->values, "ENAMETOOLONG", NUMBER_VAL(ENAMETOOLONG));
    defineNativeProperty(vm, &module->values, "ENOLCK", NUMBER_VAL(ENOLCK));
    defineNativeProperty(vm, &module->values, "ENOSYS", NUMBER_VAL(ENOSYS));
    defineNativeProperty(vm, &module->values, "ENOTEMPTY", NUMBER_VAL(ENOTEMPTY));
    defineNativeProperty(vm, &module->values, "ELOOP",  NUMBER_VAL(ELOOP));
    defineNativeProperty(vm, &module->values, "EWOULDBLOCK", NUMBER_VAL(EWOULDBLOCK));
    defineNativeProperty(vm, &module->values, "ENOMSG", NUMBER_VAL(ENOMSG));
    defineNativeProperty(vm, &module->values, "EIDRM", NUMBER_VAL(EIDRM));
#ifdef ECHRNG
    defineNativeProperty(vm, &module->values, "ECHRNG", NUMBER_VAL(ECHRNG));
#endif
#ifdef EL2NSYNC
    defineNativeProperty(vm, &module->values, "EL2NSYNC", NUMBER_VAL(EL2NSYNC));
#endif
#ifdef EL3HLT
    defineNativeProperty(vm, &module->values, "EL3HLT", NUMBER_VAL(EL3HLT));
#endif
#ifdef EL3RST
    defineNativeProperty(vm, &module->values, "EL3RST", NUMBER_VAL(EL3RST));
#endif
#ifdef ELNRNG
    defineNativeProperty(vm, &module->values, "ELNRNG", NUMBER_VAL(ELNRNG));
#endif
#ifdef EUNATCH
    defineNativeProperty(vm, &module->values, "EUNATCH", NUMBER_VAL(EUNATCH));
#endif
#ifdef ENOCSI
    defineNativeProperty(vm, &module->values, "ENOCSI", NUMBER_VAL(ENOCSI));
#endif
#ifdef EL2HLT
    defineNativeProperty(vm, &module->values, "EL2HLT", NUMBER_VAL(EL2HLT));
#endif
#ifdef EBADE
    defineNativeProperty(vm, &module->values, "EBADE", NUMBER_VAL(EBADE));
#endif
#ifdef EBADR
    defineNativeProperty(vm, &module->values, "EBADR", NUMBER_VAL(EBADR));
#endif
#ifdef EXFULL
    defineNativeProperty(vm, &module->values, "EXFULL", NUMBER_VAL(EXFULL));
#endif
#ifdef ENOANO
    defineNativeProperty(vm, &module->values, "ENOANO", NUMBER_VAL(ENOANO));
#endif
#ifdef EBADRQC
    defineNativeProperty(vm, &module->values, "EBADRQC", NUMBER_VAL(EBADRQC));
#endif
#ifdef EBADSLT
    defineNativeProperty(vm, &module->values, "EBADSLT", NUMBER_VAL(EBADSLT));
#endif
#ifdef EDEADLOCK
    defineNativeProperty(vm, &module->values, "EDEADLOCK", NUMBER_VAL(EDEADLOCK));
#endif
#ifdef EBFONT
    defineNativeProperty(vm, &module->values, "EBFONT", NUMBER_VAL(EBFONT));
#endif
    defineNativeProperty(vm, &module->values, "ENOSTR", NUMBER_VAL(ENOSTR));
    defineNativeProperty(vm, &module->values, "ENODATA", NUMBER_VAL(ENODATA));
    defineNativeProperty(vm, &module->values, "ETIME", NUMBER_VAL(ETIME));
    defineNativeProperty(vm, &module->values, "ENOSR", NUMBER_VAL(ENOSR));
#ifdef ENONET
    defineNativeProperty(vm, &module->values, "ENONET", NUMBER_VAL(ENONET));
#endif
#ifdef ENOPKG
    defineNativeProperty(vm, &module->values, "ENOPKG", NUMBER_VAL(ENOPKG));
#endif
#ifdef EREMOTE
    defineNativeProperty(vm, &module->values, "EREMOTE", NUMBER_VAL(EREMOTE));
#endif
    defineNativeProperty(vm, &module->values, "ENOLINK", NUMBER_VAL(ENOLINK));
#ifdef EADV
    defineNativeProperty(vm, &module->values, "EADV", NUMBER_VAL(EADV));
#endif
#ifdef ESRMNT
    defineNativeProperty(vm, &module->values, "ESRMNT", NUMBER_VAL(ESRMNT));
#endif
#ifdef ECOMM
    defineNativeProperty(vm, &module->values, "ECOMM", NUMBER_VAL(ECOMM));
#endif
    defineNativeProperty(vm, &module->values, "EPROTO", NUMBER_VAL(EPROTO));
#ifdef EMULTIHOP
    defineNativeProperty(vm, &module->values, "EMULTIHOP", NUMBER_VAL(EMULTIHOP));
#endif
#ifdef EDOTDOT
    defineNativeProperty(vm, &module->values, "EDOTDOT", NUMBER_VAL(EDOTDOT));
#endif
    defineNativeProperty(vm, &module->values, "EBADMSG", NUMBER_VAL(EBADMSG));
    defineNativeProperty(vm, &module->values, "EOVERFLOW", NUMBER_VAL(EOVERFLOW));
#ifdef ENOTUNIQ
    defineNativeProperty(vm, &module->values, "ENOTUNIQ", NUMBER_VAL(ENOTUNIQ));
#endif
#ifdef EBADFD
    defineNativeProperty(vm, &module->values, "EBADFD", NUMBER_VAL(EBADFD));
#endif
#ifdef EREMCHG
    defineNativeProperty(vm, &module->values, "EREMCHG", NUMBER_VAL(EREMCHG));
#endif
#ifdef ELIBACC
    defineNativeProperty(vm, &module->values, "ELIBACC", NUMBER_VAL(ELIBACC));
#endif
#ifdef ELIBBAD
    defineNativeProperty(vm, &module->values, "ELIBBAD", NUMBER_VAL(ELIBBAD));
#endif
#ifdef ELIBSCN
    defineNativeProperty(vm, &module->values, "ELIBSCN", NUMBER_VAL(ELIBSCN));
#endif
#ifdef ELIBMAX
    defineNativeProperty(vm, &module->values, "ELIBMAX", NUMBER_VAL(ELIBMAX));
#endif
#ifdef ELIBEXEC
    defineNativeProperty(vm, &module->values, "ELIBEXEC", NUMBER_VAL(ELIBEXEC));
#endif
    defineNativeProperty(vm, &module->values, "EILSEQ", NUMBER_VAL(EILSEQ));
#ifdef ERESTART
    defineNativeProperty(vm, &module->values, "ERESTART", NUMBER_VAL(ERESTART));
#endif
#ifdef ESTRPIPE
    defineNativeProperty(vm, &module->values, "ESTRPIPE", NUMBER_VAL(ESTRPIPE));
#endif
#ifdef EUSERS
    defineNativeProperty(vm, &module->values, "EUSERS", NUMBER_VAL(EUSERS));
#endif
    defineNativeProperty(vm, &module->values, "ENOTSOCK", NUMBER_VAL(ENOTSOCK));
    defineNativeProperty(vm, &module->values, "EDESTADDRREQ", NUMBER_VAL(EDESTADDRREQ));
    defineNativeProperty(vm, &module->values, "EMSGSIZE", NUMBER_VAL(EMSGSIZE));
    defineNativeProperty(vm, &module->values, "EPROTOTYPE", NUMBER_VAL(EPROTOTYPE));
    defineNativeProperty(vm, &module->values, "ENOPROTOOPT", NUMBER_VAL(ENOPROTOOPT));
    defineNativeProperty(vm, &module->values, "EPROTONOSUPPORT", NUMBER_VAL(EPROTONOSUPPORT));
#ifdef ESOCKTNOSUPPORT
    defineNativeProperty(vm, &module->values, "ESOCKTNOSUPPORT", NUMBER_VAL(ESOCKTNOSUPPORT));
#endif
    defineNativeProperty(vm, &module->values, "EOPNOTSUPP", NUMBER_VAL(EOPNOTSUPP));
#ifdef EPFNOSUPPORT
    defineNativeProperty(vm, &module->values, "EPFNOSUPPORT", NUMBER_VAL(EPFNOSUPPORT));
#endif
    defineNativeProperty(vm, &module->values, "EAFNOSUPPORT", NUMBER_VAL(EAFNOSUPPORT));
    defineNativeProperty(vm, &module->values, "EADDRINUSE", NUMBER_VAL(EADDRINUSE));
    defineNativeProperty(vm, &module->values, "EADDRNOTAVAIL", NUMBER_VAL(EADDRNOTAVAIL));
    defineNativeProperty(vm, &module->values, "ENETDOWN", NUMBER_VAL(ENETDOWN));
    defineNativeProperty(vm, &module->values, "ENETUNREACH", NUMBER_VAL(ENETUNREACH));
    defineNativeProperty(vm, &module->values, "ENETRESET", NUMBER_VAL(ENETRESET));
    defineNativeProperty(vm, &module->values, "ECONNABORTED", NUMBER_VAL(ECONNABORTED));
    defineNativeProperty(vm, &module->values, "ECONNRESET", NUMBER_VAL(ECONNRESET));
    defineNativeProperty(vm, &module->values, "ENOBUFS", NUMBER_VAL(ENOBUFS));
    defineNativeProperty(vm, &module->values, "EISCONN", NUMBER_VAL(EISCONN));
    defineNativeProperty(vm, &module->values, "ENOTCONN", NUMBER_VAL(ENOTCONN));
#ifdef ESHUTDOWN
    defineNativeProperty(vm, &module->values, "ESHUTDOWN", NUMBER_VAL(ESHUTDOWN));
#endif
#ifdef ETOOMANYREFS
    defineNativeProperty(vm, &module->values, "ETOOMANYREFS", NUMBER_VAL(ETOOMANYREFS));
#endif
    defineNativeProperty(vm, &module->values, "ETIMEDOUT", NUMBER_VAL(ETIMEDOUT));
    defineNativeProperty(vm, &module->values, "ECONNREFUSED", NUMBER_VAL(ECONNREFUSED));
#ifdef EHOSTDOWN
    defineNativeProperty(vm, &module->values, "EHOSTDOWN", NUMBER_VAL(EHOSTDOWN));
#endif
    defineNativeProperty(vm, &module->values, "EHOSTUNREACH", NUMBER_VAL(EHOSTUNREACH));
    defineNativeProperty(vm, &module->values, "EALREADY", NUMBER_VAL(EALREADY));
    defineNativeProperty(vm, &module->values, "EINPROGRESS", NUMBER_VAL(EINPROGRESS));
#ifdef ESTALE
    defineNativeProperty(vm, &module->values, "ESTALE", NUMBER_VAL(ESTALE));
#endif
#ifdef EUCLEAN
    defineNativeProperty(vm, &module->values, "EUCLEAN", NUMBER_VAL(EUCLEAN));
#endif
#ifdef ENOTNAM
    defineNativeProperty(vm, &module->values, "ENOTNAM", NUMBER_VAL(ENOTNAM));
#endif
#ifdef ENAVAIL
    defineNativeProperty(vm, &module->values, "ENAVAIL", NUMBER_VAL(ENAVAIL));
#endif
#ifdef EISNAM
    defineNativeProperty(vm, &module->values, "EISNAM", NUMBER_VAL(EISNAM));
#endif
#ifdef EREMOTEIO
    defineNativeProperty(vm, &module->values, "EREMOTEIO", NUMBER_VAL(EREMOTEIO));
#endif
#ifdef EDQUOT
    defineNativeProperty(vm, &module->values, "EDQUOT", NUMBER_VAL(EDQUOT));
#endif
#ifdef ENOMEDIUM
    defineNativeProperty(vm, &module->values, "ENOMEDIUM", NUMBER_VAL(ENOMEDIUM));
#endif
#ifdef EMEDIUMTYPE
    defineNativeProperty(vm, &module->values, "EMEDIUMTYPE", NUMBER_VAL(EMEDIUMTYPE));
#endif
    defineNativeProperty(vm, &module->values, "ECANCELED", NUMBER_VAL(ECANCELED));
#ifdef ENOKEY
    defineNativeProperty(vm, &module->values, "ENOKEY", NUMBER_VAL(ENOKEY));
#endif
#ifdef EKEYEXPIRED
    defineNativeProperty(vm, &module->values, "EKEYEXPIRED", NUMBER_VAL(EKEYEXPIRED));
#endif
#ifdef EKEYREVOKED
    defineNativeProperty(vm, &module->values, "EKEYREVOKED", NUMBER_VAL(EKEYREVOKED));
#endif
#ifdef EKEYREJECTED
    defineNativeProperty(vm, &module->values, "EKEYREJECTED", NUMBER_VAL(EKEYREJECTED));
#endif
    defineNativeProperty(vm, &module->values, "EOWNERDEAD", NUMBER_VAL(EOWNERDEAD));
    defineNativeProperty(vm, &module->values, "ENOTRECOVERABLE", NUMBER_VAL(ENOTRECOVERABLE));
#ifdef ERFKILL
    defineNativeProperty(vm, &module->values, "ERFKILL", NUMBER_VAL(ERFKILL));
#endif
#ifdef EHWPOISON
    defineNativeProperty(vm, &module->values, "EHWPOISON", NUMBER_VAL(EHWPOISON));
#endif

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
