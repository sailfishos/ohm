#ifndef __OHM_PLAYBACK_H__
#define __OHM_PLAYBACK_H__

#ifndef DEBUG
#define DEBUG(fmt, args...) do {                                        \
        printf("[%s] "fmt"\n", __FUNCTION__, ## args);                  \
    } while (0)
#endif

#endif /* __OHM_PLAYBACK_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
