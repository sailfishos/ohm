#ifndef __OHM_DBUS_INTERNAL_H
#define __OHM_DBUS_INTERNAL_H

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <ohm/ohm-plugin-dbus.h>

int  ohm_dbus_init(DBusGConnection *gconn);
void ohm_dbus_exit(void);

DBusConnection *ohm_dbus_get_connection(void);

int ohm_dbus_add_method(ohm_dbus_method_t *method);
int ohm_dbus_del_method(ohm_dbus_method_t *method);

int  ohm_dbus_add_signal(const char *sender,
                         const char *iface,
                         const char *sig,
                         const char *path,
                         DBusObjectPathMessageFunction handler,
                         void *data);

void ohm_dbus_del_signal(const char *sender,
                         const char *iface,
                         const char *sig,
                         const char *path,
                         DBusObjectPathMessageFunction handler,
                         void *data);

#endif /* __OHM_DBUS_INTERNAL_H */
