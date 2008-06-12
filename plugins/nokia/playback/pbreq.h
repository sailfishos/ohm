#ifndef __OHM_PLAYBACK_PBREQ_H__
#define __OHM_PLAYBACK_PBREQ_H__

#include <dbus/dbus.h>

/* playback request statuses */
#define PBREQ_PENDING       1
#define PBREQ_FINISHED      0
#define PBREQ_ERROR        -1

typedef enum {
    pbreq_unknown = 0,
    pbreq_queued,
    pbreq_pending,
    pbreq_handled
} pbreq_status_e;

#define PBREQ_LIST \
    struct pbreq_s  *next; \
    struct pbreq_s  *prev

typedef struct pbreq_s {
    PBREQ_LIST;
    struct client_s  *pb;
    pbreq_status_e    sts;
    DBusMessage      *msg;
    char             *state;
} pbreq_t;

typedef struct {
    PBREQ_LIST;
} pbreq_listhead_t;

static void      pbreq_init(OhmPlugin *);
static pbreq_t  *pbreq_create(struct client_s *, DBusMessage *, const char *);
static void      pbreq_destroy(pbreq_t *);
static pbreq_t  *pbreq_get_first(void);
static int       pbreq_process(void);
static void      pbreq_purge(struct client_s *);


#endif /* __OHM_PLAYBACK_PBREQ_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
