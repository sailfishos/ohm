#ifndef __OHM_LIBPLAYBACK_H__
#define __OHM_LIBPLAYBACK_H__

#include <dbus/dbus.h>


/*
 * Playback objects
 */
#define OHM_PLAYBACK_LIST \
    struct ohm_playback_s *next; \
    struct ohm_playback_s *prev

typedef struct ohm_playback_s {
    OHM_PLAYBACK_LIST;
    char           *client;       /* D-Bus id of the client */
    char           *object;       /* path of the playback object */
    char           *group;
    char           *state;
    struct {
        int play;
        int stop;
    }               allow;
} ohm_playback_t;

typedef struct {
    OHM_PLAYBACK_LIST;
} ohm_pblisthead_t;

/*
 * Request
 */
typedef enum {
    ohm_pbreq_unknown = 0,
    ohm_pbreq_queued,
    ohm_pbreq_pending,
    ohm_pbreq_handled
} ohm_pbreq_status_e;

#define OHM_PBREQ_LIST \
    struct ohm_pbreq_s  *next; \
    struct ohm_pbreq_s  *prev

typedef struct ohm_pbreq_s {
    OHM_PBREQ_LIST;
    ohm_playback_t     *pb;
    ohm_pbreq_status_e  sts;
    DBusMessage        *msg;
    char               *state;
} ohm_pbreq_t;

typedef struct {
    OHM_PBREQ_LIST;
} ohm_pbreqlisthead_t;

#endif /* __OHM_LIBPLAYBACK_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
