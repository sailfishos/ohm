#ifndef __OHM_PLAYBACK_CLIENT_H__
#define __OHM_PLAYBACK_CLIENT_H__

#include "dbusif.h"

/* FactStore prefix */
#define FACTSTORE_PREFIX                "com.nokia.policy"
#define FACTSTORE_PLAYBACK              FACTSTORE_PREFIX ".playback"

#define CLIENT_LIST \
    struct client_s *next; \
    struct client_s *prev

typedef struct client_s {
    CLIENT_LIST;
    char           *dbusid;       /* D-Bus id of the client */
    char           *object;       /* path of the playback object */
    char           *group;
    char           *state;
    struct {
        int play;
        int stop;
    }               allow;
} client_t;

typedef struct {
    CLIENT_LIST;
} client_listhead_t;




static void       client_init(OhmPlugin *);

static client_t  *client_create(char *, char *);
static void       client_destroy(client_t *);
static client_t  *client_find(char *, char *);
static void       client_purge(char *);

static int        client_add_factsore_entry(char *, char *);
static void       client_delete_factsore_entry(client_t *);
static void       client_update_factsore_entry(client_t *,char *,char *);

static void       client_get_property(client_t *, char *, get_property_cb_t);
#if 0
static void       client_set_property(playback_t *, char *, char *,
                                      set_property_cb_t);
#endif


#endif /* __OHM_PLAYBACK_CLIENT_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
