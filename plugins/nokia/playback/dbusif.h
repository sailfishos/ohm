#ifndef __OHM_PLAYBACK_DBUSIF_H__
#define __OHM_PLAYBACK_DBUSIF_H__

#include <dbus/dbus.h>

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

typedef void  (*get_property_cb_t)(char *, char *, char *, char *);
typedef void  (*set_property_cb_t)(char *, char *, int, char *);

static void dbusif_init(OhmPlugin *);
static void dbusif_reply_to_req_state(DBusMessage *, const char *);
static void dbusif_get_property(char *, char *, char *, get_property_cb_t);

#endif /*  __OHM_PLAYBACK_DBUSIF_H__ */

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

