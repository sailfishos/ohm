#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <ohm-plugin.h>
#include <dbus/dbus.h>

#include "libplayback.h"

/* D-Bus interface names */
#define DBUS_ADMIN_INTERFACE            "org.freedesktop.DBus"
#define DBUS_PLAYBACK_INTERFACE         "org.maemo.Playback"
#define DBUS_PLAYBACK_MANAGER_INTERFACE DBUS_PLAYBACK_INTERFACE ".Manager"

/* D-Bus signal & method names */
#define DBUS_NAME_OWNER_CHANGED_SIGNAL  "NameOwnerChanged"
#define DBUS_HELLO_SIGNAL               "Hello"

#define DBUS_PLAYBACK_REQUEST_METHOD    "RequestState"

/* D-Bus pathes */
#define DBUS_PLAYBACK_MANAGER_PATH     "/org/maemo/Playback/Manager"


static DBusConnection   *sess_conn;
static ohm_pblisthead_t  pb_head;

OHM_IMPORTABLE(int, resolve, (char *goal, char **locals));

static DBusHandlerResult name_changed(DBusConnection *, DBusMessage *, void *);
static DBusHandlerResult hello(DBusConnection *, DBusMessage *, void *);

static void plugin_init(OhmPlugin *);
static void plugin_destroy(OhmPlugin *);

static ohm_playback_t *playback_new(const char *, const char *);
static ohm_playback_t *playback_find(const char *, const char *);


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
                /* a libplayback client went away, unregister it */
                printf("client %s is gone\n", sender);
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

        printf("Hello from %s%s\n", sender, path);
        
        pb = playback_new(sender, path);
    }

    return result;
}

static DBusHandlerResult playback_request(DBusConnection *conn,
                                          DBusMessage    *msg,
                                          void           *user_data)
{
    static const char  *stop = "Stop";

    dbus_uint32_t      serial;
    const char        *msgpath;
    const char        *objpath;
    const char        *sender;
    const char        *state;
    ohm_playback_t    *pb;
    char              *locals[9];
    DBusMessage       *reply;
    int                success;
    DBusHandlerResult  result;
    int                i;

    success = dbus_message_is_method_call(msg, DBUS_PLAYBACK_MANAGER_INTERFACE,
                                          DBUS_PLAYBACK_REQUEST_METHOD);

    if (!success)
        result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    else {
        result = DBUS_HANDLER_RESULT_HANDLED;

        serial  = dbus_message_get_serial(msg);
        msgpath = dbus_message_get_path(msg);
        sender  = dbus_message_get_sender(msg);
        reply   = dbus_message_new_method_return(msg);

        success = dbus_message_get_args(msg, NULL,
                                        DBUS_TYPE_OBJECT_PATH, &objpath,
                                        DBUS_TYPE_STRING, &state,
                                        DBUS_TYPE_INVALID);

        if (!success)
            state = stop;
        else {
            if ((pb = playback_find(sender, objpath)) == NULL)
                state = "Stop";
            else {
                /* call the dependency resolver here */
                printf("Received playback request from %s%s %s %p\n",
                       pb->client, pb->object, state, resolve); 

                locals[i=0] = "audio_playback_request";
                locals[++i] = (char *)state;
                locals[++i] = "completion_callback";
                locals[++i] = "libplayback.completion_cb";
                locals[++i] = "audio_playback_media";
                locals[++i] = "unknown";
                locals[++i] = NULL;

                resolve("audio_playback_request", locals);

                pb->state = state;

            }
        }

        success = dbus_message_append_args(reply, DBUS_TYPE_STRING,&state,
                                           DBUS_TYPE_INVALID);
        if (!success)
            dbus_message_unref(msg);
        else {
            dbus_connection_send(sess_conn, reply, &serial);
            dbus_message_unref(reply);
        }
    }

    return result;
}

static void plugin_init(OhmPlugin *plugin)
{
#define FILTER_SIGNAL(i) "type='signal',interface='" i "'"

    static char *adm_rule = FILTER_SIGNAL(DBUS_ADMIN_INTERFACE);
    static char *pb_rule  = FILTER_SIGNAL(DBUS_PLAYBACK_INTERFACE);

    static struct DBusObjectPathVTable pb_methods = {
        .message_function = playback_request
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
    }
    if (!dbus_connection_add_filter(sess_conn, name_changed,NULL, NULL)) {
        g_error("Can't add filter 'name_changed'");
    }

    dbus_bus_add_match(sess_conn, pb_rule, &err);
    if (dbus_error_is_set(&err)) {
        g_error("Can't add match \"%s\": %s", pb_rule, err.message);
    }
    if (!dbus_connection_add_filter(sess_conn, hello,NULL, NULL)) {
        g_error("Can't add filter 'hello'");
    }

    /*
     *
     */
    success = dbus_connection_register_object_path(sess_conn,
                                                   DBUS_PLAYBACK_MANAGER_PATH,
                                                   &pb_methods, NULL);
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


#undef FILTER_SIGNAL
}

static void plugin_destroy(OhmPlugin *plugin)
{

}

static ohm_playback_t *playback_new(const char *client, const char *object)
{
    ohm_playback_t *pb = playback_find(client, object);

    if (pb == NULL) {
        if ((pb = malloc(sizeof(*pb))) != NULL) {
            memset(pb, 0, sizeof(*pb));

            pb->client = strdup(client);
            pb->object = strdup(object);

            pb_head.prev->next = pb;
            pb->next = (ohm_playback_t *)&pb_head;

            pb->prev = pb_head.prev;
            pb_head.prev = pb;
        }
    }

    return pb;
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

OHM_EXPORTABLE(void, completion_cb, (int transid, int success))
{
    printf("*** libplayback.%s(%d, %s)\n", __FUNCTION__,
           transid, success ? "OK":"FAILED");
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
