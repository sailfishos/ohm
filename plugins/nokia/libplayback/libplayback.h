#ifndef __OHM_LIBPLAYBACK_H__
#define __OHM_LIBPLAYBACK_H__

#define OHM_PLAYBACK_LIST \
    struct ohm_playback_s *next; \
    struct ohm_playback_s *prev

typedef struct ohm_playback_s {
    OHM_PLAYBACK_LIST;
    const char   *client;       /* D-Bus id of the client */
    const char   *object;       /* path of the playback object */
    const char   *state;
    struct {
        int       play;
        int       stop;
    }             allow;
} ohm_playback_t;

typedef struct {
    OHM_PLAYBACK_LIST;
} ohm_pblisthead_t;

#endif /* __OHM_LIBPLAYBACK_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
