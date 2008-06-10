/**
 * @file hal.c
 * @brief OHM HAL plugin 
 * @author ismo.h.puustinen@nokia.com
 *
 * Copyright (C) 2008, Nokia. All rights reserved.
 */

#include "hal.h"

/* this uses libhal (for now) */

/* FIXME:
 *
 * - How are the OhmFacts reference counted? We don't want to remove
 *   them if they are scheduled to be signaled
 * - How to implement changesets?
 * - Add support to adding/removing device capabilities (new fields in
 *   OhmFacts
 * - How do the 64-bit uints map to any allowed OhmFact value?
 */

typedef struct _hal_plugin {
    LibHalContext *hal_ctx;
    DBusConnection *c;
    GSList *modified_properties;
    OhmFactStore *fs;
} hal_plugin;

typedef struct _hal_modified_property {
    char *udi;
    char *key;
    dbus_bool_t is_removed;
    dbus_bool_t is_added;
} hal_modified_property;


static gchar * escape_udi(const char *hal_udi)
{
    /* returns an escaped copy of the udi */
    gchar *escaped_udi;

    /* escape the udi  */
    int i, len;
    
    if (strlen(hal_udi) < 2)
        return NULL;

    escaped_udi = g_strdup(hal_udi + 1);
    
    if (escaped_udi == NULL)
        return NULL;

    len = strlen(escaped_udi);
    
    if (len < 2) {
        g_free(escaped_udi);
        return NULL;
    }

    for (i = 1; i < len; i++) {
        if (escaped_udi[i] == '/')
            escaped_udi[i] = '.';
    }

    return escaped_udi;

}

static GValue * get_value_from_property(hal_plugin *plugin, const char *udi, const char *key) {
    /* this assumes implicit mapping between dbus and glib types */
    
    GValue *value = NULL;
    LibHalPropertyType type = libhal_device_get_property_type(plugin->hal_ctx, udi, key, NULL);

    switch (type) {
#if 0
        case LIBHAL_PROPERTY_TYPE_INT32:
            {
                dbus_int32_t hal_value = libhal_device_get_property_int(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = g_new(GValue, 1);
                g_value_init(value, G_TYPE_INT);
                g_value_set_int(value, hal_value);
                break;
            }
#if DBUS_HAVE_INT64
        case LIBHAL_PROPERTY_TYPE_UINT64:
            {
                dbus_uint64_t hal_value = libhal_device_get_property_uint64(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = g_new(GValue, 1);
                g_value_init(value, G_TYPE_UINT64);
                g_value_set_uint64(value, hal_value);
                break;
            }
#endif
        case LIBHAL_PROPERTY_TYPE_DOUBLE:
            {
                gdouble hal_value = libhal_device_get_property_double(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = g_new(GValue, 1);
                g_value_init(value, G_TYPE_DOUBLE);
                g_value_set_double(value, hal_value);
                break;
            }
        case LIBHAL_PROPERTY_TYPE_BOOLEAN:
            {
                dbus_bool_t hal_value = libhal_device_get_property_bool(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = g_new(GValue, 1);
                g_value_init(value, G_TYPE_BOOLEAN);
                g_value_set_boolean(value, hal_value);
                break;
            }
#endif
        case LIBHAL_PROPERTY_TYPE_STRING:
            {
                char *hal_value = libhal_device_get_property_string(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = ohm_str_value(hal_value);
                libhal_free_string(hal_value);
                break;
            }
        case LIBHAL_PROPERTY_TYPE_STRLIST:
            /* TODO */
            break;
        default:
            /* error case */
            /* TODO */
            break;
    }

    return value;
}

/* Helper functions to access the FactStore */

static OhmFact * get_fact(hal_plugin *plugin, const char *udi)
{
    /* Get an OhmFact that corresponds to the UDI from the 
     * FactStore. Returns NULL if the fact is not present. */

    gchar *escaped_udi = NULL;
    GSList *list = NULL;
    
    escaped_udi = escape_udi(udi);
    if (escaped_udi == NULL)
        return NULL;

    list = ohm_fact_store_get_facts_by_name(plugin->fs, escaped_udi);
    g_free(escaped_udi);

    if (g_slist_length(list) != 1) {
        /* What to do? */
        return NULL;
    }

    return list->data;
}

static gboolean set_fact(hal_plugin *plugin, OhmFact *fact)
{
    /* Inserts the OhmFact to the FactStore */

    ohm_fact_store_insert(plugin->fs, fact);
    g_print("inserted fact '%p' to FactStore\n", fact);
    /* TODO error handling */
    return TRUE;
}

static OhmFact * create_fact(hal_plugin *plugin, const char *udi, LibHalPropertySet *properties)
{
    /* Create an OhmFact based on the properties of a HAL object */

    LibHalPropertySetIterator iter;
    OhmFact *fact = NULL;

    gchar *escaped_udi = escape_udi(udi);

    if (escaped_udi == NULL)
        return NULL;

    fact = ohm_fact_new(escaped_udi);
    g_print("created fact '%s' at '%p'\n", escaped_udi, fact);
    g_free(escaped_udi);

    if (!fact)
        return NULL;
    libhal_psi_init(&iter, properties);
    
    /* TODO error handling */
    /* TODO: refactor to use only the PropertySet thingie */

    while (libhal_psi_has_more(&iter)) {
        char *key = libhal_psi_get_key(&iter);
        GValue *val = get_value_from_property(plugin, udi, key);

        if (val) {
            ohm_fact_set(fact, key, val);
            g_print("  added key '%s' with value '%s'\n",
                    key, g_value_get_string(val)); /* FIXME: tmp */
        }
        libhal_psi_next(&iter); /* FIXME: make sure we get all this way */
    }

    return fact;
}

static gboolean delete_fact(hal_plugin *plugin, OhmFact *fact) 
{
    /* Remove the OhmFact from the FactStore */

    /* TODO error handling */
    ohm_fact_store_remove(plugin->fs, fact);
    g_print("deleted fact '%p' from FactStore\n", fact);
    g_object_unref(fact);

    return TRUE;
}

static gboolean interesting(hal_plugin *plugin, const char *udi)
{
    /* see if we are interested in the OhmFact */ 
    /* TODO */
    return TRUE;
}

static void
hal_device_added_cb (LibHalContext *ctx,
        const char *udi)
{
    LibHalPropertySet *properties = NULL;
    OhmFact *fact = NULL;
    hal_plugin *plugin = (hal_plugin *) libhal_ctx_get_user_data(ctx);

    printf("> hal_device_added_cb: udi '%s'\n", udi);
    
    if (!interesting(plugin, udi))
        return;

    /* if yes, go fetch the object */
    properties = libhal_device_get_all_properties(ctx, udi, NULL);

    fact = create_fact(plugin, udi, properties);
    if (fact)
        set_fact(plugin, fact);

    libhal_free_property_set(properties);
}

static void
hal_device_removed_cb (LibHalContext *ctx,
        const char *udi)
{
    OhmFact *fact = NULL;
    hal_plugin *plugin = (hal_plugin *) libhal_ctx_get_user_data(ctx);

    printf("> hal_device_removed_cb: udi '%s'\n", udi);
    
    fact = get_fact(plugin, udi);
        
    /* TODO: see if we want to remove the OhmFact? */

    if (fact)
        delete_fact(plugin, fact);
}

static gboolean process_modified_properties(gpointer data) 
{
    hal_plugin *plugin = (hal_plugin *) data;
    GSList *e = NULL;
    g_print("> process_modified_properties\n");

    /* TODO: should we put these into an OhmFact changeset? */

    for (e = plugin->modified_properties; e != NULL; e = g_slist_next(e)) {
        hal_modified_property *modified_property = e->data;
        OhmFact *fact = get_fact(plugin, modified_property->udi);
        GValue *value = NULL;
        if (!fact) {
            /* not found */
            continue;
        }

        value = get_value_from_property(plugin, modified_property->udi, modified_property->key);
        /* this assumes implicit mapping between dbus and glib types */

        if (value)
            ohm_fact_set(fact, modified_property->key, value);

        g_free(modified_property->udi);
        g_free(modified_property->key);
        g_free(modified_property);
        e->data = NULL;
    }

    g_slist_free(plugin->modified_properties);
    plugin->modified_properties = NULL;

    /* do not call again */
    return FALSE;
}

    static void
hal_property_modified_cb (LibHalContext *ctx,
        const char *udi,
        const char *key,
        dbus_bool_t is_removed,
        dbus_bool_t is_added)
{

    /* This function is called several times when a signal that contains
     * information of multiple HAL property modifications arrives.
     * Schedule a delayed processing of data in the idle loop. */

    hal_modified_property *modified_property = NULL;
    hal_plugin *plugin = (hal_plugin *) libhal_ctx_get_user_data(ctx);

    printf("> hal_property_modified_cb: udi '%s', key '%s', %s, %s\n",
            udi,
            key,
            is_removed ? "removed" : "not removed",
            is_added ? "added" : "not added");

    if (!plugin->modified_properties) {
        g_idle_add(process_modified_properties, plugin);
    }

    modified_property = g_new0(hal_modified_property, 1);
    
    modified_property->udi = g_strdup(udi);
    modified_property->key = g_strdup(key);
    modified_property->is_removed = is_removed;
    modified_property->is_added = is_added;

    plugin->modified_properties = g_slist_prepend(plugin->modified_properties,
            modified_property);

    return;
}

/* to be refactored somewhere */
static void init_hal(DBusConnection *c)
{
    DBusError error;
    hal_plugin *plugin = g_new0(hal_plugin, 1);
    int i = 0, num_devices = 0;
    char **all_devices;

    if (!plugin) {
        return;
    }
    plugin->hal_ctx = libhal_ctx_new();
    plugin->c = c;
    plugin->fs = ohm_fact_store_get_fact_store();

    /* TODO: error handling everywhere */
    dbus_error_init(&error);

    libhal_ctx_set_dbus_connection(plugin->hal_ctx, c);

    /* start a watch on new devices */

    libhal_ctx_set_device_added(plugin->hal_ctx, hal_device_added_cb);
    libhal_ctx_set_device_removed(plugin->hal_ctx, hal_device_removed_cb);
    libhal_ctx_set_device_property_modified(plugin->hal_ctx, hal_property_modified_cb);

    libhal_ctx_set_user_data(plugin->hal_ctx, plugin);

    libhal_ctx_init(plugin->hal_ctx, &error);

    /* get all devices */

    all_devices = libhal_get_all_devices(plugin->hal_ctx, &num_devices, &error);

    for (i = 0; i < num_devices; i++) {
        /* see if the device is interesting or not */

        /* for all interesting devices */
        LibHalPropertySet *properties;
        char *udi = all_devices[i];
        OhmFact *fact = NULL;

        if (!interesting(plugin, udi))
            continue;

        libhal_device_add_property_watch(plugin->hal_ctx, udi, &error);
        properties = libhal_device_get_all_properties(plugin->hal_ctx, udi, &error);

        /* create an OhmFact based on the properties */

        fact = get_fact(plugin, udi);

        if (!fact) {
            fact = create_fact(plugin, udi, properties);
            set_fact(plugin, fact);
        }
        else {
            /* TODO: There already is a fact of this name. Add the properties to it. */
        }

        libhal_free_property_set(properties);
        libhal_free_string(udi);
    }
}

static void deinit_hal()
{
    /* TODO: deinit and free the HAL context */
    return;
}

static void
plugin_init(OhmPlugin * plugin)
{
    DBusConnection *c = ohm_plugin_dbus_get_connection();
    g_print("> HAL plugin init\n");
    /* should we ref the connection? */
    init_hal(c);
    g_print("< HAL plugin init\n");
    return;
}

static void
plugin_exit(OhmPlugin * plugin)
{
    deinit_hal();
    return;
}

OHM_PLUGIN_DESCRIPTION("hal",
        "0.0.1",
        "ismo.h.puustinen@nokia.com",
        OHM_LICENSE_NON_FREE, plugin_init, plugin_exit,
        NULL);

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
