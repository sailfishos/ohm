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

#if 0
    if ((dbus_methods = g_hash_table_new(g_str_hash, g_str_equal)) == NULL) {
        g_warning("Failed to create DBUS method hash table.");
        return FALSE;
    }
#endif    

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

    g_print("registering DBUS handler (%p) for %s.%s...\n", handler,
           path, name);

    if ((object = g_hash_table_lookup(dbus_objects, path)) == NULL) {
        g_print("No object %s found, need to register\n", path);
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
    else
        g_print("object %s found (%p)\n", path, object);
    if (g_hash_table_lookup(object->methods, name) != NULL ||
        (method = g_new0(ohm_dbus_method_t, 1)) == NULL)
        return FALSE;
    
    method->name    = name;
    method->handler = handler;
    method->data    = data;
    
    g_hash_table_insert(object->methods, (gpointer)name, method);

    g_print("registered DBUS handler %p for %s.%s...\n", handler,
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




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

