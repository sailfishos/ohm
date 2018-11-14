#ifndef __OHM_PLUGIN_DBUS_H
#define __OHM_PLUGIN_DBUS_H

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>


typedef struct {
  const char                    *interface;           /* interface */
  const char                    *path;                /* DBUS object path */
  const char                    *name;                /* method name */
  DBusObjectPathMessageFunction  handler;             /* method handler */
  void                          *data;                /* user data */
} ohm_dbus_method_t;

#define OHM_DBUS_METHODS_END { NULL, NULL, NULL, NULL, NULL }

typedef struct {
  const char                    *sender;
  const char                    *interface;
  const char                    *signal;
  const char                    *path;
  DBusObjectPathMessageFunction  handler;
  void                          *data;
} ohm_dbus_signal_t;

#define OHM_DBUS_SIGNALS_END { NULL, NULL, NULL, NULL, NULL, NULL }

DBusConnection *ohm_plugin_dbus_get_connection(void);

int ohm_plugin_dbus_add_method(ohm_dbus_method_t *method);
int ohm_plugin_dbus_del_method(ohm_dbus_method_t *method);

int  ohm_plugin_dbus_add_signal(const char *sender,
                                const char *iface,
                                const char *signal,
                                const char *path,
                                DBusObjectPathMessageFunction handler,
                                void *data);

void ohm_plugin_dbus_del_signal(const char *sender,
                                const char *iface,
                                const char *signal,
                                const char *path,
                                DBusObjectPathMessageFunction handler,
                                void *data);

#endif /* __OHM_PLUGIN_DBUS_H */
