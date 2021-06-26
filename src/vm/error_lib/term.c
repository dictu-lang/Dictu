#include <stdlib.h>
#include <string.h>

#include "term.h"

/* FIXME: These are unreliable */

bool
term_supports_utf8(void)
{
    const char *lang = getenv("LANG");
    if (lang != NULL && (strstr(lang, "UTF-8") || strstr(lang, "UTF8")))
        return true;
    return false;
}

bool
term_supports_colour(void)
{
    const char *term = getenv("TERM");
    if (term != NULL && strstr(term, "color"))
        return true;
    return false;
}
