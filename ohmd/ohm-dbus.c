#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ohm-common.h"
#include "ohm-dbus.h"

#include "ohm-dbus.h"

static DBusHandlerResult ohm_dbus_dispatch_method(DBusConnection *c,
                                                  DBusMessage *msg, void *data);

typedef struct {
    GHashTable *methods;
} ohm_dbus_object_t;



static DBusConnection *conn;
static GHashTable     *dbus_objects;


/**
 * ohm_dbus_init:
 **/
int
ohm_dbus_init(DBusGConnection *gconn)
{
    conn = dbus_g_connection_get_connection(gconn);

    if ((dbus_objects = g_hash_table_new(g_str_hash, g_str_equal)) == NULL) {
        g_warning("Failed to create DBUS object hash table.");
        return FALSE;
    }

#if 0
    if ((dbus_methods = g_hash_table_new(g_str_hash, g_str_equal)) == NULL) {
        g_warning("Failed to create DBUS method hash table.");
        return FALSE;
    }
#endif    

    return TRUE;
}


/**
 * ohm_dbus_exit:
 **/
void
ohm_dbus_exit(void)
{
    conn = NULL;
    return;
}


/**
 * ohm_dbus_get_connection:
 **/
DBusConnection *
ohm_dbus_get_connection(void)
{
    return conn;
}


/**
 * ohm_dbus_add_method:
 **/
int
ohm_dbus_add_method(const char *path, const char *name,
                    DBusObjectPathMessageFunction handler, void *data)
{
    DBusObjectPathVTable  vtable;
    ohm_dbus_object_t    *object;
    ohm_dbus_method_t    *method;

    if ((object = g_hash_table_lookup(dbus_objects, path)) == NULL) {
        if ((object = g_new0(ohm_dbus_object_t, 1)) == NULL)
            return FALSE;
        if ((object->methods = g_hash_table_new(g_str_hash,
                                                g_str_equal)) == NULL) {
            g_free(object);
            return FALSE;
        }

        vtable.message_function    = ohm_dbus_dispatch_method;
        vtable.unregister_function = NULL;
        
        if (!dbus_connection_register_object_path(conn,
                                                  path, &vtable, object)) {
            g_hash_table_destroy(object->methods);
            g_free(object);
            return FALSE;
        }

        g_hash_table_insert(dbus_objects, (gpointer)path, object);
    }
    
    if (g_hash_table_lookup(object->methods, name) != NULL ||
        (method = g_new0(ohm_dbus_method_t, 1)) == NULL)
        return FALSE;
    
    method->name    = name;
    method->handler = handler;
    method->data    = data;
    
    g_hash_table_insert(object->methods, (gpointer)name, method);

    printf("registered DBUS handler %p for %s.%s...\n", handler,
           path, name);
    
    return TRUE;
}


/**
 * ohm_dbus_del_method:
 **/
int
ohm_dbus_del_method(const char *path, const char *member,
                    DBusObjectPathMessageFunction handler, void *data)
{
    ohm_dbus_object_t *object;
    
    if ((object = g_hash_table_lookup(dbus_objects, path)) == NULL)
        return FALSE;
        
    if (g_hash_table_lookup(object->methods, member) == handler) {
        g_hash_table_remove(object->methods, member);
        return TRUE;
    }
    
    return FALSE;
}



/**
 * ohm_dbus_dispatch_method:
 **/
static DBusHandlerResult
ohm_dbus_dispatch_method(DBusConnection *c, DBusMessage *msg, void *data)
{
    ohm_dbus_object_t *object = (ohm_dbus_object_t *)data;
    ohm_dbus_method_t *method;
    const char *member    = dbus_message_get_member(msg);

#if 0
    const char *interface = dbus_message_get_interface(msg);
    const char *signature = dbus_message_get_signature(msg);
    const char *path      = dbus_message_get_path(msg);
    const char *sender    = dbus_message_get_sender(msg);

    printf("%s: got message %s.%s (%s), path=%s from %s\n", __FUNCTION__,
           interface, member, signature, path ?: "NULL", sender);
#endif
    
    if ((method = g_hash_table_lookup(object->methods, member)) != NULL)
        return method->handler(c, msg, method->data);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}



/**
 * ohm_dbus_add_signal:
 **/
int
ohm_dbus_add_signal(const char *sender, const char *interface, const char *sig,
                    const char *path,
                    DBusObjectPathMessageFunction handler, void *data)
{
#define MATCH(c, v) do {                                                \
        if ((v) != NULL) {                                              \
            p += snprintf(p, sizeof(rule) - 1 - (int)(p-rule),          \
                          "%s%s='%s'", t, (c), (v));                    \
            t=",";                                                      \
        }                                                               \
    } while (0)
    
    char  rule[DBUS_MAXIMUM_MATCH_RULE_LENGTH], *p = rule;
    char *t = "";

    MATCH("type"     , "signal");
    MATCH("sender"   , sender);
    MATCH("interface", interface);
    MATCH("signal"   , sig);
    MATCH("path"     , path);

    dbus_bus_add_match(conn, rule, NULL);
    dbus_connection_add_filter(conn, handler, data, NULL);
    
    return TRUE;
}


/**
 * ohm_dbus_del_signal:
 **/
void
ohm_dbus_del_signal(const char *sender, const char *interface, const char *sig,
                    const char *path,
                    DBusObjectPathMessageFunction handler, void *data)
{
    char  rule[DBUS_MAXIMUM_MATCH_RULE_LENGTH], *p = rule;
    char *t = "";

    MATCH("type"     , "signal");
    MATCH("sender"   , sender);
    MATCH("interface", interface);
    MATCH("signal"   , sig);
    MATCH("path"     , path);

    dbus_bus_remove_match(conn, rule, NULL);
    dbus_connection_remove_filter(conn, handler, data);
#undef MATCH
}




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

