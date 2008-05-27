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
static void ohm_dbus_purge_methods(void);
static void ohm_dbus_purge_signals(void);


typedef struct {
    GHashTable *methods;
} ohm_dbus_object_t;



static DBusConnection *conn;
static GHashTable     *dbus_objects;
static GSList *dbus_signal_handlers;



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

    /* empty list */
    dbus_signal_handlers = NULL;

    return TRUE;
}


/**
 * ohm_dbus_exit:
 **/
void
ohm_dbus_exit(void)
{
    ohm_dbus_purge_methods();
    ohm_dbus_purge_signals();

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

    g_print("registered DBUS handler %p for %s.%s...\n", handler, path, name);
    
    return TRUE;
}


/**
 * ohm_dbus_del_method:
 **/
int
ohm_dbus_del_method(const char *path, const char *name,
                    DBusObjectPathMessageFunction handler, void *data)
{
    ohm_dbus_object_t *object;
    ohm_dbus_method_t *method;

    if (dbus_objects == NULL)
        return TRUE;

    if ((object = g_hash_table_lookup(dbus_objects, path)) == NULL ||
        (method = g_hash_table_lookup(object->methods, name)) == NULL)
        return FALSE;
    
    if (method->handler != handler /* || method->data != data */) {
        g_warning("%s.%s has installed handler %p instead of %p.",
                  path, name, method->handler, handler);
        return FALSE;
    }

    g_hash_table_remove(object->methods, name);
    g_free(method);

    g_print("unregistered DBUS handler %s.%s...\n", path, name);

    if (g_hash_table_size(object->methods) == 0) {
        g_print("Object %s has no more methods, destroying it.\n", path);
        g_hash_table_destroy(object->methods);
        g_hash_table_remove(dbus_objects, path);
        g_free(object);
    }

    if (g_hash_table_size(dbus_objects) == 0) {
        g_print("No more DBUS objects.\n");
        g_hash_table_destroy(dbus_objects);
        dbus_objects = NULL;
    }
        

    return TRUE;
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

#if 1
    const char *interface = dbus_message_get_interface(msg);
    const char *signature = dbus_message_get_signature(msg);
    const char *path      = dbus_message_get_path(msg);
    const char *sender    = dbus_message_get_sender(msg);

    g_print("%s: got message %s.%s (%s), path=%s from %s\n", __FUNCTION__,
           interface, member, signature, path ?: "NULL", sender);
#endif
    
    if ((method = g_hash_table_lookup(object->methods, member)) != NULL)
        return method->handler(c, msg, method->data);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}



static gboolean
purge_dbus_method(gpointer key, gpointer data, gpointer object_path)
{
    const char        *name   = (const char *)key;
    const char        *path   = (const char *)object_path;
    ohm_dbus_method_t *method = (ohm_dbus_method_t *)data;
    
    g_print("Purging DBUS method %s.%s...\n", path, name);
    g_free(method);

    return TRUE;
}


static gboolean
purge_dbus_object(gpointer key, gpointer data, gpointer dummy)
{
    const char        *path   = (const char *)key;
    ohm_dbus_object_t *object = (ohm_dbus_object_t *)data;
    
    g_print("Purging methods of DBUS object %s...\n", path);
    g_hash_table_foreach_remove(object->methods, purge_dbus_method, key);
    g_hash_table_destroy(object->methods);
    g_free(object);
    
    if (!dbus_connection_unregister_object_path(conn, path))
        g_print("Failed to unregister DBUS connection object path %s.\n", path);
    
    return TRUE;
}


/**
 * ohm_dbus_purge_methods:
 **/
static void
ohm_dbus_purge_methods(void)
{
    if (dbus_objects != NULL) {
        g_hash_table_foreach_remove(dbus_objects, purge_dbus_object, NULL);
        g_hash_table_destroy(dbus_objects);
        dbus_objects = NULL;
    }
}



DBusHandlerResult
ohm_dbus_dispatch_signal(DBusConnection * c, DBusMessage * msg, void *data)
{

    const char *interface = dbus_message_get_interface(msg);
    const char *member    = dbus_message_get_member(msg);
    const char *path      = dbus_message_get_path(msg);

#if 0
    g_print("got signal %s.%s, path %s\n", interface ?: "NULL", member,
            path ?: "NULL");
#endif

    GSList *i = NULL;
    DBusHandlerResult retval;

    if (!interface || !member || !path) {
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    for (i = dbus_signal_handlers; i != NULL; i = g_slist_next(i)) {

        ohm_dbus_signal_t *signal_handler = i->data;

        if (!signal_handler) {
            g_print("NULL signal handler (a bug)\n");
            continue;
        }

        if (signal_handler->interface && strcmp(signal_handler->interface, interface)) {
            /* g_print("No interface match: %s vs %s\n", 
                    signal_handler->interface, interface); */
            continue;
        }
        
        if (signal_handler->signal && strcmp(signal_handler->signal, member)) {
            /* g_print("No signal name match: %s vs %s\n",
                    signal_handler->signal, member); */
            continue;
        }
        
        if (signal_handler->path && strcmp(signal_handler->path, path)) {
            /* g_print("No path match: %s vs %s\n",
                    signal_handler->path, path); */
            continue;
        }
        
        /* g_print("running the handler (%p)\n", signal_handler->handler); */
        retval = signal_handler->handler(c, msg, signal_handler->data);
    }

    return retval;
}



/**
 * ohm_dbus_add_signal:
 **/
int
ohm_dbus_add_signal(const char *sender, const char *interface, const char *sig,
                    const char *path,
                    DBusObjectPathMessageFunction handler, void *data)
{

    ohm_dbus_signal_t *signal_handler;

    g_print("> ohm_dbus_add_signal\n");
    
    if (g_slist_length(dbus_signal_handlers) == 0) {

        /* FIXME: this drains battery, since ohm is paged to memory and the
         * handler executed every time a signal is received. Refactor later. */

        g_print("registering the signal handler\n");
        dbus_bus_add_match(conn, "type='signal'", NULL);
        dbus_connection_add_filter(conn, ohm_dbus_dispatch_signal, NULL, NULL);
    
    }

    signal_handler = g_new0(ohm_dbus_signal_t, 1);
    if (signal_handler == NULL) {
        return FALSE;
    }

    signal_handler->sender = sender;
    signal_handler->interface = interface;
    signal_handler->signal = sig;
    signal_handler->path = path;
    signal_handler->handler = handler;
    signal_handler->data = data;

    dbus_signal_handlers = g_slist_prepend(dbus_signal_handlers, signal_handler);

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
    GSList *i = NULL;
    gboolean found;

    do  {
        found = FALSE;
        for (i = dbus_signal_handlers; i != NULL; i = g_slist_next(i)) {
            ohm_dbus_signal_t *signal_handler = i->data;
            if (signal_handler->handler == handler) {
                dbus_signal_handlers = g_slist_remove(
                        dbus_signal_handlers, signal_handler);

                /* free the struct */
                g_free(signal_handler);
                signal_handler = NULL;

                found = TRUE;
                break; /* for loop */
            }
        }
    } while (found);

    if (g_slist_length(dbus_signal_handlers) == 0) {
        g_print("removing signal handler\n");
        dbus_bus_remove_match(conn, "type='signal'", NULL);
        dbus_connection_remove_filter(conn, ohm_dbus_dispatch_signal, NULL);
    }
}


/**
 * ohm_dbus_purge_signals:
 **/
static void
ohm_dbus_purge_signals(void)
{
    return; /* XXX TODO */
}




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

