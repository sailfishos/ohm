/**
 * @file hal.c
 * @brief OHM HAL plugin 
 * @author ismo.h.puustinen@nokia.com
 *
 * Copyright (C) 2008, Nokia. All rights reserved.
 */

#include <ohm-plugin-debug.h>

#include "hal.h"

static int DBG_HAL, DBG_FACTS;

OHM_DEBUG_PLUGIN(hal,
    OHM_DEBUG_FLAG("hal"  , "HAL events"       , &DBG_HAL),
    OHM_DEBUG_FLAG("facts", "fact manipulation", &DBG_FACTS));

hal_plugin *hal_plugin_p;

static void
plugin_init(OhmPlugin * plugin)
{
    DBusConnection *c = ohm_plugin_dbus_get_connection();

    if (!OHM_DEBUG_INIT(hal))
        g_warning("Failed to initialize HAL plugin debugging.");

    OHM_DEBUG(DBG_HAL, "> HAL plugin init");
    /* should we ref the connection? */
    hal_plugin_p = init_hal(c, DBG_HAL, DBG_FACTS);
    OHM_DEBUG(DBG_HAL, "< HAL plugin init");
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
