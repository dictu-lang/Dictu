#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "term.h"

struct TextAttr
{
    const char *name;
    const char *attr;
};

static size_t
log_fmt_parse(const size_t dst_size, char *dst,
              const size_t src_size, const char *src)
{
    size_t log_fmt_size = 0;

    size_t src_i = 0;
    for (size_t dst_i = 0; dst_i < dst_size && src_i < src_size; dst_i++, src_i++)
    {
        if (src[src_i] == '{')
        {
            const size_t start = ++src_i;

            while (src_i < src_size && src[src_i++] != '}')
                ; /* Find the closing brace */

            const size_t end = --src_i;

#ifdef _WIN32
            static const struct TextAttr attrs[] = {
                { "reset",     ""  },
                { "bold",      ""  },
                { "dim",       ""  },
                { "italic",    ""  },
                { "underline", ""  },
                { "black",     ""  },
                { "red",       ""  },
                { "green",     ""  },
                { "yellow",    ""  },
                { "blue",      ""  },
                { "magenta",   ""  },
                { "cyan",      ""  },
                { "white",     ""  }
            };
#else // _WIN32
            static const struct TextAttr attrs[] = {
                { "reset",     "\033[0m"  },
                { "bold",      "\033[1m"  },
                { "dim",       "\033[2m"  },
                { "italic",    "\033[3m"  },
                { "underline", "\033[4m"  },
                { "black",     "\033[30m" },
                { "red",       "\033[31m" },
                { "green",     "\033[32m" },
                { "yellow",    "\033[33m" },
                { "blue",      "\033[34m" },
                { "magenta",   "\033[35m" },
                { "cyan",      "\033[36m" },
                { "white",     "\033[37m" }
            };
#endif // _WIN32

            for (size_t i = 0; i < (sizeof(attrs) / sizeof(*(attrs))); i++)
            {
                if (!strncmp(attrs[i].name, &src[start], end - start))
                {
                    strcpy(&dst[dst_i], attrs[i].attr);
                    const size_t attr_len = strlen(attrs[i].attr);
                    dst_i += attr_len - 1;
                    log_fmt_size += attr_len;
                    break;
                }
            }
        }
        else
        {
            dst[dst_i] = src[src_i];
            dst[dst_i + 1] = '\0';
        }
    }

    return log_fmt_size;
}

static size_t
internal_prettyvlog(FILE *f, const char *fmt, va_list args)
{
    assert(f != NULL);
    assert(fmt != NULL);

    const size_t fmt_len = strlen(fmt);
    const size_t size = (fmt_len * 2) + 7;
    char *buf = malloc(size);

    buf[0] = '\0';
    const size_t log_fmt_size = log_fmt_parse(size, buf, fmt_len, fmt);
    buf[size - 1] = '\0';

    const size_t vfprintf_printed = vfprintf(f, buf, args);

    free(buf);

    return vfprintf_printed - log_fmt_size;
}

static size_t
internal_prettylog(FILE *f, const char *fmt, ...)
{
    assert(f != NULL);
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    const size_t printed_len = internal_prettyvlog(f, fmt, args);
    va_end(args);

    return printed_len;
}

void
vlog(const enum LogType type, const char *fmt, va_list args, const bool newline)
{
    assert(fmt != NULL);

    static const struct TextAttr prefix[] = {
        [LOG_NONE]    = { "",            ""                          },
        [LOG_INFO]    = { "Info",        "{white}%s:{reset} "        },
        [LOG_NOTE]    = { "Note",        "{dim}%s: "                 },
        [LOG_PAD]     = { "",            ""                          },
        [LOG_DEBUG]   = { "DEBUG",       "{bold}{white}%s:{reset} "  },
        [LOG_SUCCESS] = { "Success",     "(bold}{green}%s:{reset} "  },
        [LOG_WARNING] = { "Warning",     "{bold}{yellow}%s:{reset} " },
        [LOG_ERROR]   = { "Error",       "{bold}{red}%s:{reset} "    },
        [LOG_FATAL]   = { "Fatal Error", "{bold}{red}%s:{reset} "    },
        [LOG_BUG]     = { "BUG",         "{bold}{red}%s:{reset} "    }
    };

    static size_t last_printed_length = 0;

    if (type == LOG_NONE)
        {}
    else if (type == LOG_PAD)
        last_printed_length = internal_prettylog(stderr, "%*.s", last_printed_length, "");
    else
        last_printed_length = internal_prettylog(stderr, prefix[type].attr, prefix[type].name);

    internal_prettyvlog(stderr, fmt, args);

    if (newline)
        fputc('\n', stderr);
}

static void
internal_log(const enum LogType type, const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(type, fmt, args, true);
    va_end(args);
}

void
log_none(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_NONE, fmt, args, false);
    va_end(args);
}

void
log_noneln(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_NONE, fmt, args, true);
    va_end(args);
}

void
log_info(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_INFO, fmt, args, true);
    va_end(args);
}

void
log_note(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_NOTE, fmt, args, true);
    va_end(args);
}

void
log_pad(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_PAD, fmt, args, false);
    va_end(args);
}

void
log_padln(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_PAD, fmt, args, true);
    va_end(args);
}

void
log_debug(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_DEBUG, fmt, args, true);
    va_end(args);
}

void
log_success(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_SUCCESS, fmt, args, true);
    va_end(args);
}

void
log_warning(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_WARNING, fmt, args, true);
    va_end(args);
}

void
log_error(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_ERROR, fmt, args, true);
    va_end(args);
}

void
log_fatal(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_FATAL, fmt, args, true);
    va_end(args);

    internal_log(LOG_FATAL, "Exiting due to previous error.");
    exit(EXIT_FAILURE);
}

void
log_bug(const char *fmt, ...)
{
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    vlog(LOG_BUG, fmt, args, true);
    va_end(args);

    internal_log(LOG_BUG, "Aborting due to previous error.");
    abort();
}
