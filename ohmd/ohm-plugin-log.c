#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "ohm/ohm-plugin-log.h"


static int level_mask;


/**
 * ohm_log_enabled:
 **/
int
ohm_log_enabled(OhmLogLevel level)
{
    return level_mask & OHM_LOG_LEVEL_MASK(level);
}


/**
 * ohm_log_enable:
 **/
void
ohm_log_enable(OhmLogLevel level)
{
    level_mask |= OHM_LOG_LEVEL_MASK(level);
}


/**
 * ohm_log:
 **/
void
ohm_log(OhmLogLevel level, const gchar *format, ...)
{
    va_list     ap;
    FILE       *out;
    const char *prefix;
    
    if (!(level_mask & OHM_LOG_LEVEL_MASK(level)))
        return;
    
    switch (level) {
    case OHM_LOG_ERROR:   prefix = "E: "; out = stderr; break;
    case OHM_LOG_WARNING: prefix = "W: "; out = stderr; break;
    case OHM_LOG_INFO:    prefix = "I: "; out = stdout; break;
    default:                                           return;
    }

    va_start(ap, format);

    fputs(prefix, out);
    vfprintf(out, format, ap);
    fputs("\n", out);

    va_end(ap);
}

/**
 * ohm_log_init:
 * @mask: mask of message levels we should be printing
 **/
void
ohm_log_init(int mask)
{
    if (mask != 0)
        level_mask = mask;
    else
        level_mask = OHM_LOG_LEVEL_MASK(OHM_LOG_ERROR);
}




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
