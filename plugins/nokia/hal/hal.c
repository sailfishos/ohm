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

hal_plugin *hal_plugin_p;

static void
plugin_init(OhmPlugin * plugin)
{
    DBusConnection *c = ohm_plugin_dbus_get_connection();
    g_print("> HAL plugin init\n");
    /* should we ref the connection? */
    hal_plugin_p = init_hal(c);
    g_print("< HAL plugin init\n");
    return;
}

static void
plugin_exit(OhmPlugin * plugin)
{
    if (hal_plugin_p) {
        deinit_hal(hal_plugin_p);
    }
    g_free(hal_plugin_p);
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
