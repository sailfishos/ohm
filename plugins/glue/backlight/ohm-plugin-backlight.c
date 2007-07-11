/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2
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

#include <ohm-plugin.h>
#include <dbus/dbus-glib.h>

#define	HAL_DBUS_SERVICE		 	"org.freedesktop.Hal"
#define	HAL_DBUS_INTERFACE_LAPTOP_PANEL	 	"org.freedesktop.Hal.Device.LaptopPanel"

enum {
	CONF_BRIGHTNESS_PERCENT_CHANGED,
	CONF_BRIGHTNESS_HARDWARE_CHANGED,
	CONF_LAST
};

typedef struct {
	gint levels;
} OhmPluginCacheData;

static OhmPluginCacheData data;

static gboolean
backlight_set_brightness (OhmPlugin *plugin, guint brightness)
{
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error;
	gboolean ret;
	gint retval;
	gchar *udi;

	/* get udi and connection, assume connected */
	udi = ohm_plugin_hal_get_udi (plugin);
	if (udi == NULL) {
		g_warning ("cannot set brightness as no device!");
		return FALSE;
	}
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, NULL);

	/* reuse the connection from HAL */
	proxy = dbus_g_proxy_new_for_name (connection,
					   HAL_DBUS_SERVICE, udi,
					   HAL_DBUS_INTERFACE_LAPTOP_PANEL);

	/* get the brightness from HAL */
	error = NULL;
	ret = dbus_g_proxy_call (proxy, "SetBrightness", &error,
				 G_TYPE_INT, (int)brightness,
				 G_TYPE_INVALID,
				 G_TYPE_UINT, &retval,
				 G_TYPE_INVALID);
	if (error != NULL) {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}
	g_object_unref (proxy);
	g_free (udi);
	return ret;
}

#if 0
static gboolean
backlight_get_brightness (OhmPlugin *plugin, guint *brightness)
{
	DBusConnection *connection;
	DBusGProxy *proxy;
	GError *error;
	gboolean ret;
	int level = 0;

	/* reuse the connection from HAL */
	connection = libhal_ctx_get_dbus_connection (ctx);
	proxy = dbus_g_proxy_new_for_name (connection, HAL_DBUS_SERVICE, udi, HAL_DBUS_INTERFACE_LAPTOP_PANEL);
	if (udi == NULL) {
		g_warning ("cannot get brightness as no device!");
		return FALSE;
	}

	/* get the brightness from HAL */
	error = NULL;
	ret = dbus_g_proxy_call (proxy, "GetBrightness", &error,
				 G_TYPE_INVALID,
				 G_TYPE_UINT, &level,
				 G_TYPE_INVALID);
	if (error != NULL) {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
		level = 0;
	}
	*brightness = level;
	g_object_unref (proxy);
	return ret;
}
#endif

/**
 * plugin_preload:
 * @plugin: This class instance
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static gboolean
plugin_preload (OhmPlugin *plugin)
{
	/* todo: split into dcon glue plugin */
	ohm_plugin_conf_provide (plugin, "backlight.state");

	/* tell ohmd what keys we are going to provide so it can create them */
	ohm_plugin_conf_provide (plugin, "backlight.hardware_brightness");
	ohm_plugin_conf_provide (plugin, "backlight.percent_brightness");
	ohm_plugin_conf_provide (plugin, "backlight.num_levels");
	return TRUE;
}

static guint
percent_to_discrete (guint percentage,
		     guint levels)
{
	/* check we are in range */
	if (percentage > 100) {
		return levels;
	}
	if (levels == 0) {
		g_warning ("levels is 0!");
		return 0;
	}
	return ((gfloat) percentage * (gfloat) (levels - 1)) / 100.0f;
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
	gboolean ret;

	/* interested keys, either can be changed without changing the other  */
	ohm_plugin_conf_interested (plugin, "backlight.hardware_brightness", CONF_BRIGHTNESS_HARDWARE_CHANGED);
	ohm_plugin_conf_interested (plugin, "backlight.percent_brightness", CONF_BRIGHTNESS_PERCENT_CHANGED);

	/* initialise HAL */
	ohm_plugin_hal_init (plugin);

	/* get the only device with capability and watch it */
	ret = ohm_plugin_hal_add_device_capability (plugin, "laptop_panel");
	if (ret == TRUE) {
		ohm_plugin_hal_get_int (plugin, "laptop_panel.num_levels", &data.levels);
		g_debug ("Laptop panel levels: %i", data.levels);
	} else {
		g_warning ("not tested with not one laptop_panel");
	}

	/* get levels that the adapter supports -- this does not change ever */
	ohm_plugin_hal_get_int (plugin, "laptop_panel.num_levels", &data.levels);
	if (data.levels == 0) {
		g_error ("levels zero!");
		return;
	}
	ohm_plugin_conf_set_key (plugin, "backlight.num_levels", data.levels);
}

/**
 * plugin_conf_notify:
 * @plugin: This class instance
 *
 * Notify the plugin that a key marked with ohm_plugin_conf_interested ()
 * has it's value changed.
 * An enumerated numeric id rather than the key is returned for processing speed.
 */
static void
plugin_conf_notify (OhmPlugin *plugin, gint id, gint value)
{
	guint hw;
	if (id == CONF_BRIGHTNESS_PERCENT_CHANGED) {
		hw = percent_to_discrete (value, data.levels);
		ohm_plugin_conf_set_key (plugin, "backlight.hardware_brightness", hw);
		// backlight_set_brightness (plugin, hw); ------ SHOULDN'T BE NEEDED
	} else if (id == CONF_BRIGHTNESS_HARDWARE_CHANGED) {
		backlight_set_brightness (plugin, value);
	}
}

static OhmPluginInfo plugin_info = {
	"OHM Backlight",		/* description */
	"0.0.2",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	NULL,				/* unload */
	plugin_coldplug,		/* coldplug */
	plugin_conf_notify,		/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
