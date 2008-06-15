/**
 * @file hal-internal.c
 * @brief OHM HAL plugin private functions
 * @author ismo.h.puustinen@nokia.com
 *
 * Copyright (C) 2008, Nokia. All rights reserved.
 */

/* this uses libhal (for now) */

#include "hal.h"

/* FIXME:
 *
 * - How are the OhmFacts reference counted? We don't want to remove
 *   them if they are scheduled to be signaled or something
 * - Add support to adding/removing device capabilities (new fields in
 *   OhmFacts
 * - How do the 64-bit uints map to any allowed OhmFact types?
 */


typedef struct _hal_modified_property {
    char *udi;
    char *key;
    dbus_bool_t is_removed;
    dbus_bool_t is_added;
} hal_modified_property;

static gchar * escape_udi(const char *hal_udi)
{
    /* returns an escaped copy of the udi:
     *
     * /org/freedesktop/key becomes
     * org.freedesktop.key
     *
     * */
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
        case LIBHAL_PROPERTY_TYPE_INT32:
            {
                dbus_int32_t hal_value = libhal_device_get_property_int(
                        plugin->hal_ctx,
                        udi,
                        key,
                        NULL);

                value = ohm_int_value(hal_value);
                break;
            }
#if 0
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
        case LIBHAL_PROPERTY_TYPE_STRLIST:
            /* TODO */
            break;
#endif
        default:
            /* error case */
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
        g_print("The requested fact was not found\n");
        return NULL;
    }

    return list->data;
}

static gboolean set_fact(hal_plugin *plugin, OhmFact *fact)
{
    /* Inserts the OhmFact to the FactStore */

    g_print("inserting fact '%p' to FactStore\n", fact);
    return ohm_fact_store_insert(plugin->fs, fact);
}

static OhmFact * create_fact(hal_plugin *plugin, const char *udi, LibHalPropertySet *properties)
{
    /* Create an OhmFact based on the properties of a HAL object */

    LibHalPropertySetIterator iter;
    OhmFact *fact = NULL;
    int i, len;
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

    len = libhal_property_set_get_num_elems(properties);

    for (i = 0; i < len; i++, libhal_psi_next(&iter)) {
        char *key = libhal_psi_get_key(&iter);
        LibHalPropertyType type = libhal_psi_get_type(&iter);
        GValue *val = NULL;

        /* Not good to duplicate the switch, consider strategy pattern. Still,
         * it is a good idea to fetch the properties only once. */
        switch (type) {
            case LIBHAL_PROPERTY_TYPE_INT32:
                {
                    dbus_int32_t hal_value = libhal_psi_get_int(&iter);
                    val = ohm_int_value(hal_value);
                    break;
                }
            case LIBHAL_PROPERTY_TYPE_STRING:
                {
                    /* freed with propertyset*/
                    char *hal_value = libhal_psi_get_string(&iter);
                    val = ohm_str_value(hal_value);
                    break;
                }
            default:
                /* error case, currently means that FactStore doesn't
                 * support the type yet */
                break;
        }

        if (val) {
            ohm_fact_set(fact, key, val);
        }
    }

    return fact;
}

static gboolean delete_fact(hal_plugin *plugin, OhmFact *fact) 
{
    /* Remove the OhmFact from the FactStore */

    ohm_fact_store_remove(plugin->fs, fact);
    g_print("deleted fact '%p' from FactStore\n", fact);
    g_object_unref(fact);

    /* we don't get a return value fro ohm_fact_store_unref */
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

    ohm_fact_store_transaction_push(plugin->fs); /* begin transaction */

    for (e = plugin->modified_properties; e != NULL; e = g_slist_next(e)) {
        hal_modified_property *modified_property = e->data;
        OhmFact *fact = get_fact(plugin, modified_property->udi);
        GValue *value = NULL;
        if (!fact) {
            g_print("No fact found to be modified, most likely unsupported type\n");
        }
        else {
            value = get_value_from_property(plugin, modified_property->udi, modified_property->key);

            if (value) {
                /* FIXED: Do we need to free the original value or does the
                 * setter do it automatically? Apparently the setter
                 * does it. */
                ohm_fact_set(fact, modified_property->key, value);
            }
        }

        g_free(modified_property->udi);
        g_free(modified_property->key);
        g_free(modified_property);
        e->data = NULL;
    }

    ohm_fact_store_transaction_pop(plugin->fs, FALSE); /* commit */

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

hal_plugin * init_hal(DBusConnection *c)
{
    DBusError error;
    hal_plugin *plugin = g_new0(hal_plugin, 1);
    int i = 0, num_devices = 0;
    char **all_devices;

    if (!plugin) {
        return NULL;
    }
    plugin->hal_ctx = libhal_ctx_new();
    plugin->c = c;
    plugin->fs = ohm_fact_store_get_fact_store();

    /* TODO: error handling everywhere */
    dbus_error_init(&error);

    if (!libhal_ctx_set_dbus_connection(plugin->hal_ctx, c))
        goto error;

    /* start a watch on new devices */

    if (!libhal_ctx_set_device_added(plugin->hal_ctx, hal_device_added_cb))
        goto error;
    if (!libhal_ctx_set_device_removed(plugin->hal_ctx, hal_device_removed_cb))
        goto error;
    if (!libhal_ctx_set_device_property_modified(plugin->hal_ctx, hal_property_modified_cb))
        goto error;

    if (!libhal_ctx_set_user_data(plugin->hal_ctx, plugin))
        goto error;

    if (!libhal_ctx_init(plugin->hal_ctx, &error))
        goto error;

    /* get all devices */

    all_devices = libhal_get_all_devices(plugin->hal_ctx, &num_devices, &error);

    for (i = 0; i < num_devices; i++) {
        /* see if the device is interesting or not */

        /* for all interesting devices */
        LibHalPropertySet *properties;
        char *udi = all_devices[i];
        OhmFact *fact = NULL;

        if (!udi || !interesting(plugin, udi))
            continue;

        properties = libhal_device_get_all_properties(plugin->hal_ctx, udi, &error);

        if (properties) {

            /* We got properties, so let's start listening for them */
            libhal_device_add_property_watch(plugin->hal_ctx, udi, &error);

            /* create an OhmFact based on the properties */
            fact = get_fact(plugin, udi);

            if (!fact) {
                fact = create_fact(plugin, udi, properties);
                set_fact(plugin, fact);
            }
            else {
                /* TODO: There already is a fact of this name. Add the properties to it.
                 * Do this when the add_device_capability support actually
                 * comes in. Do we honor the values already in the fact or
                 * overwrite them? */
            }
            libhal_free_property_set(properties);
        }

        libhal_free_string(udi);
    }

    return plugin;

error:

    if (dbus_error_is_set(&error)) {
        g_print("Error initializing the HAL plugin. '%s': '%s'\n", error.name, error.message);
    }
    else {
        g_print("Error initializing the HAL plugin\n");
    }

    return NULL;
}

void deinit_hal(hal_plugin *plugin)
{
    libhal_ctx_shutdown(plugin->hal_ctx, NULL);
    libhal_ctx_free(plugin->hal_ctx);
    return;
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */
