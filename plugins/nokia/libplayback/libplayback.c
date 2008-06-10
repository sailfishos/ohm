#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <ohm-plugin.h>

#include "prolog/ohm-fact.h"

#include "libplayback.h"

#ifndef DEBUG
#define DEBUG(fmt, args...) do {                                        \
        printf("[%s] "fmt"\n", __FUNCTION__, ## args);                  \
    } while (0)
#endif


/* D-Bus service names */
#define DBUS_PLAYBACK_SERVICE          "org.maemo.Playback"

/* D-Bus interface names */
#define DBUS_ADMIN_INTERFACE            "org.freedesktop.DBus"
#define DBUS_PLAYBACK_INTERFACE         "org.maemo.Playback"
#define DBUS_PLAYBACK_MANAGER_INTERFACE DBUS_PLAYBACK_INTERFACE ".Manager"

/* D-Bus signal & method names */
#define DBUS_NAME_OWNER_CHANGED_SIGNAL  "NameOwnerChanged"
#define DBUS_HELLO_SIGNAL               "Hello"
#define DBUS_NOTIFY_SIGNAL              "Notify"

#define DBUS_PLAYBACK_REQ_STATE_METHOD  "RequestState"


/* D-Bus pathes */
#define DBUS_PLAYBACK_MANAGER_PATH      "/org/maemo/Playback/Manager"

/* FactStore prefix */
#define FACTSTORE_PREFIX                "com.nokia.policy"
#define FACTSTORE_PLAYBACK              FACTSTORE_PREFIX ".playback"

/* playback request statuses */
#define PBREQ_PENDING       1
#define PBREQ_FINISHED      0
#define PBREQ_ERROR        -1

typedef void  (*get_property_cb_t)(ohm_playback_t *,const char *,const char *);
typedef void  (*set_property_cb_t)(ohm_playback_t *,int,const char *);

typedef struct {
    char              *client;
    char              *object;
    char              *prname;
    get_property_cb_t  usercb;
} property_cb_data_t;

static DBusConnection      *sess_conn;
static ohm_pblisthead_t     pb_head;
static ohm_pbreqlisthead_t  rq_head;

OHM_IMPORTABLE(int, resolve, (char *goal, char **locals));

static DBusHandlerResult name_changed(DBusConnection *, DBusMessage *, void *);
static DBusHandlerResult hello(DBusConnection *, DBusMessage *, void *);
static DBusHandlerResult notify(DBusConnection *, DBusMessage *, void *);
static void              set_group_cb(ohm_playback_t *, const char *,
                                      const char *);
static DBusHandlerResult playback_req_state(DBusConnection *, DBusMessage *,
                                            void *);

static void plugin_init(OhmPlugin *);
static void plugin_destroy(OhmPlugin *);

static ohm_playback_t *playback_create(const char *, const char *);
static void            playback_destroy(ohm_playback_t *);
static ohm_playback_t *playback_find(const char *, const char *);
static void            playback_purge(const char *);
static int             playback_add_factsore_entry(const char *, const char *);
static void            playback_delete_factsore_entry(ohm_playback_t *);
static OhmFact        *playback_find_factstore_entry(ohm_playback_t *);
static void            playback_update_factsore_entry(ohm_playback_t *, char *,
                                                      char *);
static void            playback_get_property(ohm_playback_t *, char *,
                                             get_property_cb_t);
static void            playback_get_property_cb(DBusPendingCall *, void *);
static void            playback_free_get_property_cb_data(void *);
#if 0
static void            playback_set_property(ohm_playback_t *, char *, char *,
                                             set_property_cb_t);
#endif

static ohm_pbreq_t *pbreq_create(ohm_playback_t *, DBusMessage *, const char*);
static void         pbreq_destroy(ohm_pbreq_t *);
static ohm_pbreq_t *pbreq_get_first(void);
static int          pbreq_process(void);
static void         pbreq_reply(DBusMessage *, const char *);
static void         pbreq_purge(ohm_playback_t *);

static int          playback_resolve_request(ohm_playback_t *, char *, int);


static DBusHandlerResult name_changed(DBusConnection *conn, DBusMessage *msg,
                                      void *user_data)
{
    char              *sender;
    char              *before;
    char              *after;
    gboolean           success;
    DBusHandlerResult  result;

    success = dbus_message_is_signal(msg, DBUS_ADMIN_INTERFACE,
                                     DBUS_NAME_OWNER_CHANGED_SIGNAL);

    if (!success)
        result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    else {
        result = DBUS_HANDLER_RESULT_HANDLED;

        success = dbus_message_get_args(msg, NULL,
                                        DBUS_TYPE_STRING, &sender,
                                        DBUS_TYPE_STRING, &before,
                                        DBUS_TYPE_STRING, &after,
                                        DBUS_TYPE_INVALID);

        if (success && sender != NULL && before != NULL) {
            if (!after || !strcmp(after, "")) {
                DEBUG("client %s is gone", sender);
                playback_purge(sender);
            }
        }
    }

    return result;
}

static DBusHandlerResult hello(DBusConnection *conn, DBusMessage *msg,
                               void *user_data)
{
    const char        *path;
    const char        *sender;
    ohm_playback_t    *pb;
    int                success;
    DBusHandlerResult  result;

    success = dbus_message_is_signal(msg, DBUS_PLAYBACK_INTERFACE,
                                     DBUS_HELLO_SIGNAL);

    if (!success)
        result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    else {
        result = DBUS_HANDLER_RESULT_HANDLED;

        path   = dbus_message_get_path(msg);
        sender = dbus_message_get_sender(msg);

        DEBUG("Hello from %s%s", sender, path);
        
        pb = playback_create(sender, path);

        playback_get_property(pb, "Class", set_group_cb);
    }

    return result;
}

static DBusHandlerResult notify(DBusConnection *conn, DBusMessage *msg,
                                void *user_data)
{
    const char        *path;
    const char        *sender;
    const char        *iface;
    const char        *prop;
    const char        *value;
    ohm_playback_t    *pb;
    int                success;
    DBusError          err;
    DBusHandlerResult  result;

    success = dbus_message_is_signal(msg, DBUS_INTERFACE_PROPERTIES,
                                     DBUS_NOTIFY_SIGNAL);

    if (!success)
        result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    else {
        result = DBUS_HANDLER_RESULT_HANDLED;

        path   = dbus_message_get_path(msg);
        sender = dbus_message_get_sender(msg);

        if ((pb = playback_find(sender, path)) != NULL) {
            dbus_error_init(&err);

            success = dbus_message_get_args(msg, &err,
                                            DBUS_TYPE_STRING, &iface,
                                            DBUS_TYPE_STRING, &prop,
                                            DBUS_TYPE_STRING, &value,
                                            DBUS_TYPE_INVALID);
            if (!success) {
                if (!dbus_error_is_set(&err))
                    DEBUG("Malformed Notify from %s%s", sender, path);
                else {
                    DEBUG("Malformed Notify from %s%s: %s",
                          sender, path, err.message); 
                    dbus_error_free(&err);
                }
            }
            else {
                if (!strcmp(iface, DBUS_PLAYBACK_INTERFACE)) {
                    if (!strcmp(prop, "State")) {
                        DEBUG("State of %s%s is '%s'", sender, path, value);

                    }
                }
            }
        } /* if playback_find() */
    }

    return result;
}

void set_group_cb(ohm_playback_t *pb, const char *prname, const char *prvalue)
{
    static struct {char *klass; char *group;}  map[] = {
        {"None"      , "othermedia"},
        {"Test"      , "othermedia"},
        {"Event"     , "ringtone"  },
        {"VoIP"      , "ipcall"    },
        {"Media"     , "player"    },
        {"Background", "othermedia"},
        {NULL        , "othermedia"}
    };

    int i;

    for (i = 0;   map[i].klass != NULL;   i++) {
        if (!strcmp(prvalue, map[i].klass))
            break;
    }

    pb->group = strdup(map[i].group);
    playback_update_factsore_entry(pb, "group", pb->group);

    DEBUG("playback group is set to %s", pb->group);

    pbreq_process();
}


static DBusHandlerResult playback_req_state(DBusConnection *conn,
                                            DBusMessage    *msg,
                                            void           *user_data)
{
    static const char  *stop = "Stop";

    const char        *msgpath;
    const char        *objpath;
    const char        *sender;
    const char        *state;
    ohm_playback_t    *pb;
    ohm_pbreq_t       *req;
    int                success;
    DBusHandlerResult  result;

    success = dbus_message_is_method_call(msg, DBUS_PLAYBACK_MANAGER_INTERFACE,
                                          DBUS_PLAYBACK_REQ_STATE_METHOD);

    if (!success)
        result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    else {
        result = DBUS_HANDLER_RESULT_HANDLED;

        msgpath = dbus_message_get_path(msg);
        sender  = dbus_message_get_sender(msg);
        req     = NULL;

        success = dbus_message_get_args(msg, NULL,
                                        DBUS_TYPE_OBJECT_PATH, &objpath,
                                        DBUS_TYPE_STRING, &state,
                                        DBUS_TYPE_INVALID);
        if (success) {
            if ((pb  = playback_find(sender, objpath)) != NULL &&
                (req = pbreq_create(pb, msg, state))   != NULL    ) {

                    if (pbreq_process() != PBREQ_ERROR ||
                        req->sts != ohm_pbreq_handled)
                        return result;
            }
        }

        pbreq_reply(msg, stop);
        pbreq_destroy(req);     /* copes with NULL */
    }

    return result;
}

static void plugin_init(OhmPlugin *plugin)
{
#define FILTER_SIGNAL(i) "type='signal',interface='" i "'"

    static char *adm_rule  = FILTER_SIGNAL(DBUS_ADMIN_INTERFACE);
    static char *pb_rule   = FILTER_SIGNAL(DBUS_PLAYBACK_INTERFACE);
    static char *prop_rule = FILTER_SIGNAL(DBUS_INTERFACE_PROPERTIES);

    static struct DBusObjectPathVTable req_state_method = {
        .message_function = playback_req_state
    };


    DBusError  err;
    int        retval;
    int        success;

    dbus_error_init(&err);

    /*
     * setup sess_conn
     */

    if ((sess_conn = dbus_bus_get(DBUS_BUS_SESSION, &err)) == NULL) {
        if (dbus_error_is_set(&err))
            g_error("Can't get D-Bus connection: %s", err.message);
        else
            g_error("Can't get D-Bus connection");
    }

    dbus_connection_setup_with_g_main(sess_conn, NULL);

    /*
     * add signal filters
     */

    dbus_bus_add_match(sess_conn, adm_rule, &err);
    if (dbus_error_is_set(&err)) {
        g_error("Can't add match \"%s\": %s", adm_rule, err.message);
        dbus_error_free(&err);
    }
    if (!dbus_connection_add_filter(sess_conn, name_changed,NULL, NULL)) {
        g_error("Can't add filter 'name_changed'");
    }

    dbus_bus_add_match(sess_conn, pb_rule, &err);
    if (dbus_error_is_set(&err)) {
        g_error("Can't add match \"%s\": %s", pb_rule, err.message);
        dbus_error_free(&err);
    }
    if (!dbus_connection_add_filter(sess_conn, hello,NULL, NULL)) {
        g_error("Can't add filter 'hello'");
    }

    dbus_bus_add_match(sess_conn, prop_rule, &err);
    if (dbus_error_is_set(&err)) {
        g_error("Can't add match \"%s\": %s", prop_rule, err.message);
        dbus_error_free(&err);
    }
    if (!dbus_connection_add_filter(sess_conn, notify,NULL, NULL)) {
        g_error("Can't add filter 'notify'");
    }

    /*
     *
     */
    success = dbus_connection_register_object_path(sess_conn,
                                                   DBUS_PLAYBACK_MANAGER_PATH,
                                                   &req_state_method, NULL);
    if (!success) {
        g_error("Can't register object path %s", DBUS_PLAYBACK_MANAGER_PATH);
    }

    retval = dbus_bus_request_name(sess_conn, DBUS_PLAYBACK_MANAGER_INTERFACE,
                                   DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (retval != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        if (dbus_error_is_set(&err)) {
            g_error("Can't be the primary owner for name %s: %s",
                    DBUS_PLAYBACK_MANAGER_INTERFACE, err.message);
        }
        else {
            g_error("Can't be the primary owner for name %s",
                    DBUS_PLAYBACK_MANAGER_INTERFACE);
        }
    }

    /*
     *
     */
    pb_head.next = (ohm_playback_t *)&pb_head;
    pb_head.prev = (ohm_playback_t *)&pb_head;

    rq_head.next = (ohm_pbreq_t *)&rq_head;
    rq_head.prev = (ohm_pbreq_t *)&rq_head;


#undef FILTER_SIGNAL
}

static void plugin_destroy(OhmPlugin *plugin)
{

}

static ohm_playback_t *playback_create(const char *client, const char *object)
{
    ohm_playback_t *pb = playback_find(client, object);
    ohm_playback_t *next;
    ohm_playback_t *prev;

    if (pb == NULL) {
        if ((pb = malloc(sizeof(*pb))) != NULL) {
            memset(pb, 0, sizeof(*pb));

            pb->client = strdup(client);
            pb->object = strdup(object);

            next = (ohm_playback_t *)&pb_head;
            prev = pb_head.prev;

            prev->next = pb;
            pb->next   = next;

            next->prev = pb;
            pb->prev   = prev;

            if (playback_add_factsore_entry(client, object))
                DEBUG("playback %s%s created", client, object);
            else {
                playback_destroy(pb);
                pb = NULL;
            }
        }
    }

    return pb;
}

static void playback_destroy(ohm_playback_t *pb)
{
    ohm_playback_t *prev, *next;

    if (pb != NULL) {
        DEBUG("playback %s%s going to be destroyed", pb->client, pb->object);

        playback_delete_factsore_entry(pb);
        pbreq_purge(pb);

        if (pb->client)
            free(pb->client);

        if (pb->object)
            free(pb->object);

        if (pb->group)
            free(pb->group);

        next = pb->next;
        prev = pb->prev;

        prev->next = pb->next;
        next->prev = pb->prev;

        free(pb);
    }
}

static ohm_playback_t *playback_find(const char *client, const char *object)
{
    ohm_playback_t *pb;

    for (pb = pb_head.next; pb != (ohm_playback_t *)&pb_head; pb = pb->next){
        if (!strcmp(client, pb->client) &&
            !strcmp(object, pb->object)   )
            return pb;
    }
    
    return NULL;
}

static void playback_purge(const char *client)
{
    ohm_playback_t *pb, *nxpb;

    for (pb = pb_head.next;  pb != (ohm_playback_t *)&pb_head;  pb = nxpb) {
        nxpb = pb->next;

        if (!strcmp(client, pb->client))
            playback_destroy(pb);
    }
}

static int playback_add_factsore_entry(const char *client, const char *object)
{
    OhmFactStore   *fs;
    OhmFact        *fact;
    GValue          gval;

    fs = ohm_fact_store_get_fact_store();
    fact = ohm_fact_new(FACTSTORE_PLAYBACK);

    gval = ohm_value_from_string(client);
    ohm_fact_set(fact, "client", &gval);
    
    gval = ohm_value_from_string(object);
    ohm_fact_set(fact, "object", &gval);

    gval = ohm_value_from_string("othermedia");
    ohm_fact_set(fact, "group", &gval);

    gval = ohm_value_from_string("none");
    ohm_fact_set(fact, "state", &gval);

    if (ohm_fact_store_insert(fs, fact))
        DEBUG("factstore entry %s created", FACTSTORE_PLAYBACK);
    else {
        DEBUG("Can't add %s to factsore", FACTSTORE_PLAYBACK);
        return FALSE;
    }
    
    return TRUE;
}

static void playback_delete_factsore_entry(ohm_playback_t *pb)
{
    OhmFactStore *fs;
    OhmFact      *fact;

    fs = ohm_fact_store_get_fact_store();

    if ((fact = playback_find_factstore_entry(pb)) != NULL) {
        ohm_fact_store_remove(fs, fact);

        g_object_unref(fact);

        DEBUG("Factstore entry %s deleted", FACTSTORE_PLAYBACK);
    }
}

static OhmFact *playback_find_factstore_entry(ohm_playback_t *pb)
{
    OhmFactStore *fs;
    OhmFact      *fact;
    GSList       *list;
    GValue       *gval;

    if (pb != NULL) {
        fs = ohm_fact_store_get_fact_store();

        for (list  = ohm_fact_store_get_facts_by_name(fs, FACTSTORE_PLAYBACK);
             list != NULL;
             list  = g_slist_next(list))
        {
            fact = (OhmFact *)list->data;

            if ((gval = ohm_fact_get(fact, "client")) == NULL ||
                strcmp(pb->client, g_value_get_string(gval))     )
                continue;
                
            if ((gval = ohm_fact_get(fact, "object")) == NULL ||
                strcmp(pb->object, g_value_get_string(gval))     )
                continue;
            
            return fact;
        }
    }

    return NULL;
}

static void playback_update_factsore_entry(ohm_playback_t *pb, char *member,
                                           char *value)
{
    OhmFact *fact;
    GValue   gval;

    if ((fact = playback_find_factstore_entry(pb)) && member && value) {

        gval = ohm_value_from_string(value);
        ohm_fact_set(fact, member, &gval);

        DEBUG("Factstore entry update %s[client:%s,object:%s].%s = %s",
              FACTSTORE_PLAYBACK, pb->client, pb->object, member, value);
    }
}

static void playback_get_property(ohm_playback_t *pb, char *prname,
                                  get_property_cb_t usercb)
{
    static char        *ifname = DBUS_PLAYBACK_INTERFACE;

    DBusMessage        *msg;
    DBusPendingCall    *pend;
    property_cb_data_t *ud;
    int                 success;

    msg = dbus_message_new_method_call(DBUS_PLAYBACK_SERVICE,
                                       pb->object,
                                       DBUS_INTERFACE_PROPERTIES,
                                       "Get");
    if (msg == NULL) {
        DEBUG("Failed to create D-Dbus message to request properties");
        return;
    }

    success = dbus_message_append_args(msg,
                                       DBUS_TYPE_STRING, &ifname,
                                       DBUS_TYPE_STRING, &prname,
                                       DBUS_TYPE_INVALID);
    if (!success) {
        DEBUG("Can't setup D-Bus message to get properties");
        goto failed;
    }
    
    if ((ud = malloc(sizeof(*ud))) == NULL) {
        DEBUG("Failed to allocate memory for callback data");
        goto failed;
    }

    memset(ud, 0, sizeof(*ud));
    ud->client = strdup(pb->client);
    ud->object = strdup(pb->object);
    ud->prname = strdup(prname);
    ud->usercb = usercb;

    success = dbus_connection_send_with_reply(sess_conn, msg, &pend, 1000);
    if (!success) {
        DEBUG("Failed to query properties");
        goto failed;
    }

    success = dbus_pending_call_set_notify(pend, playback_get_property_cb, ud,
                                           playback_free_get_property_cb_data);
    if (!success) {
        DEBUG("Can't set notification for pending call");
    }

    return;

 failed:
    dbus_message_unref(msg);
    return;
}

static void playback_get_property_cb(DBusPendingCall *pend, void *data)
{
    property_cb_data_t *cbd = (property_cb_data_t *)data;
    DBusMessage        *reply;
    ohm_playback_t     *pb;
    const char         *prvalue;
    int                 success;

    if ((reply = dbus_pending_call_steal_reply(pend)) == NULL || cbd == NULL) {
        DEBUG("Property receiving failed: invalid argument");
        return;
    }

    if ((pb = playback_find(cbd->client, cbd->object)) == NULL) {
        DEBUG("Property receiving failed: playback is gone");
        return;
    }

    success = dbus_message_get_args(reply, NULL,
                                    DBUS_TYPE_STRING, &prvalue,
                                    DBUS_TYPE_INVALID);
    if (!success) {
        DEBUG("Failed to parse property reply message");
        return;
    }

    DEBUG("Received property %s=%s", cbd->prname, prvalue);

    if (cbd->usercb != NULL)
        cbd->usercb(pb, cbd->prname, prvalue);

    dbus_message_unref(reply);
}

static void playback_free_get_property_cb_data(void *memory)
{
    property_cb_data_t *cbd = (property_cb_data_t *)memory;

    DEBUG("Freeing get property callback data");

    if (cbd != NULL) {
        free(cbd->client);
        free(cbd->object);
        free(cbd->prname);

        free(cbd);
    }
}

#if 0
static void playback_set_property(ohm_playback_t *pb, char *prname,
                                  char *prvalue, set_property_cb_t usercb)
{
    static char        *ifname = DBUS_PLAYBACK_INTERFACE;

    DBusMessage        *msg;
    DBusPendingCall    *pend;
    int                 success;

    msg = dbus_message_new_method_call(DBUS_PLAYBACK_SERVICE,
                                       pb->object,
                                       DBUS_INTERFACE_PROPERTIES,
                                       "Set");
    if (msg == NULL) {
        DEBUG("Failed to create D-Dbus message to set properties");
        return;
    }

    success = dbus_message_append_args(msg,
                                       DBUS_TYPE_STRING, &ifname,
                                       DBUS_TYPE_STRING, &prname,
                                       DBUS_TYPE_STRING, &prvalue,
                                       DBUS_TYPE_INVALID);
    if (!success) {
        DEBUG("Can't setup D-Bus message to set properties");
        goto failed;
    }
    
    success = dbus_connection_send_with_reply(sess_conn, msg, &pend, 1000);
    if (!success) {
        DEBUG("Failed to set properties");
        goto failed;
    }

    success = dbus_pending_call_set_notify(pend, playback_get_property_cb, ud,
                                           playback_free_set_property_cb_data);
    if (!success) {
        DEBUG("Can't set notification for pending call");
    }

    return;

 failed:
    dbus_message_unref(msg);
    return;
}
#endif


static ohm_pbreq_t *pbreq_create(ohm_playback_t *pb, DBusMessage *msg,
                                 const char *state)
{
    ohm_pbreq_t *req, *next, *prev;

    if (msg == NULL)
        req = NULL;
    else {
        if ((req = malloc(sizeof(*req))) != NULL) {
            dbus_message_ref(msg);

            memset(req, 0, sizeof(*req));            
            req->pb    = pb;
            req->sts   = ohm_pbreq_queued;
            req->msg   = msg;
            req->state = strdup(state);

            next = (ohm_pbreq_t *)&rq_head;
            prev = rq_head.prev;
            
            prev->next = req;
            req->next  = next;

            next->prev = req;
            req->prev  = prev;
        }
    }

    return req;
}

static void pbreq_destroy(ohm_pbreq_t *req)
{
    ohm_pbreq_t *prev, *next;

    if (req != NULL) {
        prev = req->prev;
        next = req->next;

        prev->next = req->next;
        next->prev = req->prev;

        if (req->msg != NULL)
            dbus_message_unref(req->msg);

        if (req->state)
            free(req->state);

        free(req);
    }
}

static ohm_pbreq_t *pbreq_get_first(void)
{
    ohm_pbreq_t *req = rq_head.next;

    if (req == (ohm_pbreq_t *)&rq_head)
        req = NULL;

    return req;
}

static int pbreq_process(void)
{
    ohm_pbreq_t    *req;
    ohm_playback_t *pb;
    int             sts;
    char            state[64];
    char           *p, *q, *e, c;

    if ((req = pbreq_get_first()) == NULL)
        sts = PBREQ_PENDING;
    else {
        pb = req->pb;

        if (pb->group == NULL)
            sts = PBREQ_PENDING;
        else {
            p = req->state;
            e = (q = state) + sizeof(state) - 1;
            while ((c = *p++) && q < e)
                *q++ = tolower(c);
            *q = '\0';

            switch (req->sts) {

            case ohm_pbreq_queued:
                if (playback_resolve_request(pb, state, TRUE)) {
                    sts = PBREQ_PENDING;
                    req->sts = ohm_pbreq_pending;
                }
                else {
                    sts = PBREQ_ERROR;
                    req->sts = ohm_pbreq_handled;
                }
                break;

            case ohm_pbreq_pending:
                sts = PBREQ_PENDING;
                break;
                
            case ohm_pbreq_handled:
                sts = PBREQ_FINISHED;
                break;
                
            default:
                sts = PBREQ_ERROR;
                break;
            }
        }
    }

    return sts;
}

static void pbreq_reply(DBusMessage *msg, const char *state)
{
    DBusMessage       *reply;
    dbus_uint32_t      serial;
    int                success;

    serial = dbus_message_get_serial(msg);
    reply  = dbus_message_new_method_return(msg);

    success = dbus_message_append_args(reply, DBUS_TYPE_STRING,&state,
                                       DBUS_TYPE_INVALID);
    if (!success)
        dbus_message_unref(msg);
    else {
        DEBUG("replying to playback request with '%s'", state);

        dbus_connection_send(sess_conn, reply, &serial);
#if 0
        dbus_message_unref(reply);
#endif
    }
}

static void pbreq_purge(ohm_playback_t *pb)
{
    ohm_pbreq_t *req, *nxreq;

    for (req = rq_head.next; req != (ohm_pbreq_t *)&rq_head; req = nxreq) {
        nxreq = req->next;

        if (pb == req->pb)
            pbreq_destroy(req);
    }
}

static int playback_resolve_request(ohm_playback_t *pb, char *state, int cb)
{
    char *vars[32];
    int   i;
    int   err;

    playback_update_factsore_entry(pb, "state", state);

    vars[i=0] = "playback_state";
    vars[++i] = state;
    vars[++i] = "playback_group";
    vars[++i] = pb->group;
    vars[++i] = "playback_media";
    vars[++i] = "unknown";

    if (cb) {
        vars[++i] = "completion_callback";
        vars[++i] = "libplayback.completion_cb";
    }

    vars[++i] = NULL;

    if ((err = resolve("audio_playback_request", vars)) != 0)
        DEBUG("resolve() failed: (%d) %s", err, strerror(err));

    return err ? FALSE : TRUE;
}

OHM_EXPORTABLE(void, completion_cb, (int transid, int success))
{
    static char    *stop = "Stop";

    ohm_pbreq_t    *req;
    ohm_playback_t *pb;

    printf("*** libplayback.%s(%d, %s)\n", __FUNCTION__,
           transid, success ? "OK":"FAILED");

    if ((req = pbreq_get_first()) == NULL) {
        DEBUG("libplayback completion_cb: Can't find the request");
        return;
    }

    if (req->sts != ohm_pbreq_pending) {
        DEBUG("libplayback completion_cb: bad state %d", req->sts);
        return;
    }

    pb = req->pb;
    
    if (pb->state != NULL)
        free(pb->state);
    pb->state = strdup(success ? req->state : stop);

    pbreq_reply(req->msg, pb->state);
    pbreq_destroy(req);
}


OHM_PLUGIN_REQUIRES_METHODS(libplayback, 1, 
   OHM_IMPORT("dres.resolve", resolve)
);

OHM_PLUGIN_PROVIDES_METHODS(libplayback, 1,
   OHM_EXPORT(completion_cb, "completion_cb")
);

OHM_PLUGIN_DESCRIPTION(
    "OHM media playback",             /* description */
    "0.0.1",                          /* version */
    "janos.f.kovacs@nokia.com",       /* author */
    OHM_LICENSE_NON_FREE,             /* license */
    plugin_init,                      /* initalize */
    plugin_destroy,                   /* destroy */
    NULL                              /* notify */
);

OHM_PLUGIN_PROVIDES(
    "maemo.playback"
);


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
