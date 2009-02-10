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
#include "ohm-debug.h"
#include <ohm/ohm-dbus.h>

#define DBUS_SIGNAL_MATCH "type='signal'"
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

    if ((dbus_objects = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL)) == NULL) {
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
        if ((object->methods = g_hash_table_new_full(g_str_hash,
                                                g_str_equal,
                                                NULL, NULL)) == NULL) {
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

    ohm_debug("registered DBUS handler %p for %s.%s...", handler, path, name);

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

    ohm_debug("unregistered DBUS handler %s.%s...", path, name);

    if (g_hash_table_size(object->methods) == 0) {
        ohm_debug("Object %s has no more methods, destroying it.", path);
        g_hash_table_destroy(object->methods);
        g_hash_table_remove(dbus_objects, path);
        g_free(object);
    }

    if (g_hash_table_size(dbus_objects) == 0) {
        ohm_debug("No more DBUS objects.");
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

    ohm_debug("%s: got message %s.%s (%s), path=%s from %s", __FUNCTION__,
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
    
    ohm_debug("Purging DBUS method %s.%s...", path, name);
    g_free(method);

    return TRUE;
}


static gboolean
purge_dbus_object(gpointer key, gpointer data, gpointer dummy)
{
    const char        *path   = (const char *)key;
    ohm_dbus_object_t *object = (ohm_dbus_object_t *)data;
    
    ohm_debug("Purging methods of DBUS object %s...", path);
    g_hash_table_foreach_remove(object->methods, purge_dbus_method, key);
    g_hash_table_destroy(object->methods);
    g_free(object);
    
    if (!dbus_connection_unregister_object_path(conn, path))
        ohm_debug("Failed to unregister DBUS object path %s.", path);
    
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

#if 1
    ohm_debug("got signal %s.%s, path %s", interface ?: "NULL", member,
              path ?: "NULL");
#endif

    GSList *i = NULL;
    DBusHandlerResult retval = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    
    if (!interface || !member || !path) {
        /* FIXME: it kind of depends what we should return here */
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    for (i = dbus_signal_handlers; i != NULL; i = g_slist_next(i)) {

        ohm_dbus_signal_t *signal_handler = i->data;

        if (!signal_handler) {
            ohm_debug("NULL signal handler (a bug)");
            continue;
        }

        if (signal_handler->interface && strcmp(signal_handler->interface, interface)) {
            /* ohm_debug("No interface match: %s vs %s", 
                         signal_handler->interface, interface); */
            continue;
        }
        
        if (signal_handler->signal && strcmp(signal_handler->signal, member)) {
            /* ohm_debug("No signal name match: %s vs %s",
                          signal_handler->signal, member); */
            continue;
        }
        
        if (signal_handler->path && strcmp(signal_handler->path, path)) {
            /* ohm_debug("No path match: %s vs %s",
                         signal_handler->path, path); */
            continue;
        }
        
        /* ohm_debug("running the handler (%p)", signal_handler->handler); */
        retval = signal_handler->handler(c, msg, signal_handler->data);
    }

    return retval;
}

static gchar * ohm_generate_match(ohm_dbus_signal_t *signal_handler)
{
    gchar *new_match_string = NULL, *match_string = g_strdup("type='signal'");

    if (match_string == NULL)
        return NULL;

    if (signal_handler->interface) {
        gchar *interface = g_strdup_printf("interface='%s'", signal_handler->interface);
        new_match_string = g_strjoin(",", match_string, interface, NULL);
        g_free(match_string);
        g_free(interface);
        match_string = new_match_string;
    }

    if (signal_handler->signal) {
        gchar *member = g_strdup_printf("member='%s'", signal_handler->signal);
        new_match_string = g_strjoin(",", match_string, member, NULL);
        g_free(match_string);
        g_free(member);
        match_string = new_match_string;
    }
    
    if (signal_handler->path) {
        gchar *path = g_strdup_printf("path='%s'", signal_handler->path);
        new_match_string = g_strjoin(",", match_string, path, NULL);
        g_free(match_string);
        g_free(path);
        match_string = new_match_string;
    }

    return match_string;


}

static gboolean unset_match_string(ohm_dbus_signal_t *signal_handler) {

    gchar *match_string = ohm_generate_match(signal_handler);
    gboolean retval = TRUE;
    DBusError error;

    if (!match_string)
        return FALSE;

    dbus_error_init(&error);
    dbus_bus_remove_match(conn, match_string, &error);

    if (dbus_error_is_set(&error)) {
        ohm_debug("Error removing D-Bus match '%s': '%s'", match_string, error.message);
        retval = FALSE;
    }
    else {
        ohm_debug("Unset match '%s'", match_string);
    }

    dbus_error_free(&error);
    g_free(match_string);

    return retval;
}

static gboolean set_match_string(ohm_dbus_signal_t *signal_handler) {

    gchar *match_string = ohm_generate_match(signal_handler);
    gboolean retval = TRUE;
    DBusError error;

    if (!match_string)
        return FALSE;

    dbus_error_init(&error);
    dbus_bus_add_match(conn, match_string, &error);

    if (dbus_error_is_set(&error)) {
        ohm_debug("Error setting D-Bus match '%s': '%s'", match_string, error.message);
        retval = FALSE;
    }
    else {
        ohm_debug("Set match '%s'", match_string);
    }

    dbus_error_free(&error);
    g_free(match_string);

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

    ohm_debug("Adding DBUS signal handler %p for %s...", handler, interface);
    
    if (g_slist_length(dbus_signal_handlers) == 0) {
        ohm_debug("Registering the signal handler.");
        dbus_connection_add_filter(conn, ohm_dbus_dispatch_signal, conn, NULL);
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

    return set_match_string(signal_handler);
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

    ohm_debug("Deleting DBUS signal handler %p for %s...", handler,interface);

    do  {
        found = FALSE;
        for (i = dbus_signal_handlers; i != NULL; i = g_slist_next(i)) {
            ohm_dbus_signal_t *signal_handler = i->data;
            if (signal_handler->handler == handler) {
                dbus_signal_handlers = g_slist_remove(
                        dbus_signal_handlers, signal_handler);

                /* free the struct */

                unset_match_string(signal_handler);
                g_free(signal_handler);
                signal_handler = NULL;

                found = TRUE;
                break; /* for loop */
            }
        }
    } while (found);

    if (g_slist_length(dbus_signal_handlers) == 0) {
        dbus_connection_remove_filter(conn, ohm_dbus_dispatch_signal, conn);
        ohm_debug("No more DBUS signals.");
    }
    
    return;     
}


/**
 * ohm_dbus_purge_signals:
 **/
static void
ohm_dbus_purge_signals(void)
{
    GSList *i = NULL;

    if (!dbus_signal_handlers)
        return;

    for (i = dbus_signal_handlers; i != NULL; i = g_slist_next(i)) {
        ohm_dbus_signal_t *signal_handler = i->data;
        unset_match_string(signal_handler);
        g_free(signal_handler);
        signal_handler = NULL;
    }
    g_slist_free(dbus_signal_handlers);

    dbus_connection_remove_filter(conn, ohm_dbus_dispatch_signal, conn);

    return;
}




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

