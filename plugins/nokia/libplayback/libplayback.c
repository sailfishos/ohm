#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <ohm-plugin.h>
#include <dbus/dbus.h>


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

static DBusHandlerResult name_changed(DBusConnection *, DBusMessage *, void *);
static DBusHandlerResult hello(DBusConnection *, DBusMessage *, void *);

static void plugin_init(OhmPlugin *);
static void plugin_destroy(OhmPlugin *);


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
        
        if (success && sender != NULL && before != NULL && after != NULL) {
            if (!strcmp(after, "")) {
                /* a libplayback client went away, unregister it */
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

        /* ??? */

    }

    return result;
}

static void plugin_init(OhmPlugin *plugin)
{
    DBusConnection  *conn = ohm_plugin_dbus_get_connection();

    if ((conn = ohm_plugin_dbus_get_connection()) == NULL) {
        g_error("Can't get D-Bus connection");
    }
}

static void plugin_destroy(OhmPlugin *plugin)
{

}

OHM_PLUGIN_DBUS_SIGNALS(
    {NULL,                            /* sender */
     DBUS_ADMIN_INTERFACE,            /* interface */
     DBUS_NAME_OWNER_CHANGED_SIGNAL,  /* signal */
     NULL,                            /* path */
     name_changed,                    /* handler */
     NULL                             /* user_data */
    },

    {NULL,                            /* sender */
     DBUS_PLAYBACK_INTERFACE,         /* interface */
     DBUS_HELLO_SIGNAL,               /* signal */
     NULL,                            /* path */
     hello,                           /* handler */
     NULL                             /* user_data */
    } 
);

#if 0
OHM_PLUGIN_DBUS_METHODS(
    {DBUS_
);
#endif

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
