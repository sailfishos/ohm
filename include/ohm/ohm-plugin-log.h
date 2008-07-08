#ifndef __OHM_PLUGIN_LOG_H__
#define __OHM_PLUGIN_LOG_H__

#define OHM_MESSAGE(type, fmt, args...)        \
    do {                                       \
        printf("["#type"] "fmt"\n", ## args);  \
    } while (0)

#define OHM_ERROR(fmt, args...)   OHM_MESSAGE(ERROR, fmt, ## args)
#define OHM_INFO(fmt, args...)    OHM_MESSAGE(INFO, fmt, ## args)
#define OHM_WARNING(fmt, args...) OHM_MESSAGE(WARNING, fmt, ## args)

#endif /* __OHM_PLUGIN_DEBUG_H__ */

