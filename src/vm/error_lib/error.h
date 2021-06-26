#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

// TODO: Dump command line, version, build options when BUG() is called

#define LOG_BUG(...) log_bug(__VA_ARGS__)
#define LOG_DEBUG(...) log_debug(__VA_ARGS__)

enum LogType
{
    LOG_NONE,
    LOG_INFO,
    LOG_NOTE,
    LOG_PAD,
    LOG_DEBUG,
    LOG_SUCCESS,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_BUG
};

void vlog(const enum LogType type, const char *fmt, va_list args, const bool newline);
void log_none(const char *fmt, ...);
void log_noneln(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_note(const char *fmt, ...);
void log_pad(const char *fmt, ...);
void log_padln(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_success(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_fatal(const char *fmt, ...);
void log_bug(const char *fmt, ...);

#endif /* __ERROR_H__ */

// info() success() note() warning() error() fatal() bug() debug() pad() - to previous error prefix length
// logging_disable_colour(), !isatty(stdout)
// logging_disable_utf8(), !strstr(getenv("LANG"), "UTF-8")
// {bold}, {dim}, {italic}, {underline}, {reset}
// {black}, {red}, {green}, {yellow}, {blue}, {magenta}, {cyan}, {white}
