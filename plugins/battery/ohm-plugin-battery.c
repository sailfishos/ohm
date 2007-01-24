/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <gmodule.h>
#include <glib.h>
#include <libhal.h>
#include <string.h>

#include <ohm-plugin.h>

typedef struct {
	LibHalContext *ctx;
	gchar *udi;
} OhmPluginCacheData;

OhmPluginCacheData data;
OhmPlugin *plugin_global; /* ick, needed as there is no userdata with libhal */

/**
 * plugin_load:
 * @plugin: This class instance
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static void
plugin_load (OhmPlugin *plugin)
{
	/* tell ohmd what keys we are going to provide - don't set keys
	 * unless you provide them */
	ohm_plugin_conf_provide (plugin, "battery.percentage");
	plugin_global = plugin;
}

/**
 * plugin_unload:
 * @plugin: This class instance
 *
 * Called just before the plugin module is unloaded, and gives the plugin
 * a chance to free private memory.
 */
static void
plugin_unload (OhmPlugin *plugin)
{
	if (data.udi != NULL) {
		libhal_device_remove_property_watch (data.ctx, data.udi, NULL);
		g_free (data.udi);
	}
	libhal_ctx_shutdown (data.ctx, NULL);
}

static void
hal_property_changed_cb (LibHalContext *ctx,
			 const char *udi,
			 const char *key,
			 dbus_bool_t is_removed,
			 dbus_bool_t is_added)
{
	gboolean state;
	if (strcmp (key, "battery.charge_level.percentage") == 0) {
		state = libhal_device_get_property_int (ctx, udi, key, NULL);
		ohm_plugin_conf_set_key (plugin_global, "battery.percentage", state);
	}
}

/**
 * plugin_coldplug:
 * @plugin: This class instance
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_coldplug (OhmPlugin *plugin)
{
	char **devices;
	int num_devices;
	int state;
	DBusConnection *conn;

	conn = dbus_bus_get (DBUS_BUS_SYSTEM, NULL);
	
	data.ctx = libhal_ctx_new ();
	libhal_ctx_set_dbus_connection (data.ctx, conn);
	libhal_ctx_init (data.ctx, NULL);
	libhal_ctx_set_device_property_modified (data.ctx, hal_property_changed_cb);

	devices = libhal_find_device_by_capability (data.ctx, "battery", &num_devices, NULL);
	if (num_devices == 1) {
		data.udi = g_strdup (devices[0]);
		libhal_device_add_property_watch (data.ctx, data.udi, NULL);
		state = libhal_device_get_property_int (data.ctx, data.udi, "battery.charge_level.percentage", NULL);
		ohm_plugin_conf_set_key (plugin, "battery.percentage", state);
	} else {
		data.udi = NULL;
		ohm_plugin_conf_set_key (plugin, "battery.percentage", 100);
		g_error ("not tested with not one battery");
	}
	libhal_free_string_array (devices);
}

static OhmPluginInfo plugin_info = {
	"OHM HAL Battery",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_load,			/* load */
	plugin_unload,			/* unload */
	plugin_coldplug,		/* coldplug */
	NULL,				/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
