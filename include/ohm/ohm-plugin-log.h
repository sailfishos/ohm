#ifndef __OHM_PLUGIN_LOG_H__
#define __OHM_PLUGIN_LOG_H__

#if 1

G_BEGIN_DECLS

#define OHM_MESSAGE(level, fmt, args...) \
  ohm_log(OHM_LOG_##level, fmt, ## args)

#define OHM_ERROR(fmt, args...)   OHM_MESSAGE(ERROR, fmt, ## args)
#define OHM_INFO(fmt, args...)    OHM_MESSAGE(INFO, fmt, ## args)
#define OHM_WARNING(fmt, args...) OHM_MESSAGE(WARNING, fmt, ## args)

#define OHM_LOGGED(level) ohm_log_enabled(OHM_LOG_##level)

typedef enum {
  OHM_LOG_NONE    = 0,
  OHM_LOG_ERROR   = 1,
  OHM_LOG_WARNING = 2,
  OHM_LOG_INFO    = 3,
  OHM_LOG_DEBUG   = 4,
} OhmLogLevel;

#define OHM_LOG_ALL 0xffffffff

#define OHM_LOG_LEVEL_MASK(level) (1 << ((level) - 1))

void ohm_log_init   (int level_mask);
void ohm_log        (OhmLogLevel level, const gchar *format, ...);
int  ohm_log_enable (OhmLogLevel level);
int  ohm_log_disable(OhmLogLevel level);
int  ohm_log_enabled(OhmLogLevel level);

G_END_DECLS

#else

#define OHM_MESSAGE(type, fmt, args...)        \
    do {                                       \
        printf("["#type"] "fmt"\n", ## args);  \
    } while (0)

#define OHM_ERROR(fmt, args...)   OHM_MESSAGE(ERROR, fmt, ## args)
#define OHM_INFO(fmt, args...)    OHM_MESSAGE(INFO, fmt, ## args)
#define OHM_WARNING(fmt, args...) OHM_MESSAGE(WARNING, fmt, ## args)
#endif

#endif /* __OHM_PLUGIN_LOG_H__ */

