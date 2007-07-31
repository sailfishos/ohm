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
#define	HAL_DBUS_INTERFACE_POWER	 	"org.freedesktop.Hal.Device.SystemPowerManagement"
#define HAL_ROOT_COMPUTER		 	"/org/freedesktop/Hal/devices/computer"

enum {
	CONF_BUTTON_POWER_CHANGED,
	CONF_BUTTON_LID_CHANGED,
	CONF_IDLE_POWERDOWN_CHANGED,
	CONF_INHIBIT_CHANGED,
	CONF_LAST
};

static gboolean
system_suspend (OhmPlugin *plugin)
{
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error;
	gboolean ret;
	gint retval;

	/* Tell the DPMS plugin to kill the backlight. */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 0);

	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, NULL);

	/* reuse the connection from HAL */
	proxy = dbus_g_proxy_new_for_name (connection,
					   HAL_DBUS_SERVICE,
					   HAL_ROOT_COMPUTER,
					   HAL_DBUS_INTERFACE_POWER);

	/* get the brightness from HAL */
	error = NULL;
	ret = dbus_g_proxy_call (proxy, "Suspend", &error,
				 G_TYPE_INT, 0,
				 G_TYPE_INVALID,
				 G_TYPE_UINT, &retval,
				 G_TYPE_INVALID);
	if (error != NULL) {
		g_printerr ("Error: %s\n", error->message);
		g_error_free (error);
	}
	g_object_unref (proxy);

	/* We've resumed now.  Bring the backlight back. */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 1);

	return ret;
}

static void
power_button_pressed (OhmPlugin *plugin)
{
	gint value;

	/* check system inhibit - this is broken as any client can unref all */
	ohm_plugin_conf_get_key (plugin, "suspend.fixme_inhibit", &value);
	if (value == 1) {
		g_warning ("not doing lid closed action as inhibited");
		return;
	}

	/* actually do the suspend */
	system_suspend (plugin);
}

static void
lid_closed (OhmPlugin *plugin, gboolean is_closed)
{
	gint value;

	if (is_closed == FALSE) {
		return;
	}

	/* check system inhibit - this is broken as any client can unref all */
	ohm_plugin_conf_get_key (plugin, "suspend.fixme_inhibit", &value);
	if (value == 1) {
		g_warning ("not doing lid closed action as inhibited");
		return;
	}

	/* actually do the suspend */
	system_suspend (plugin);
}

static void
system_is_idle (OhmPlugin *plugin)
{
	gint value;

	/* check system inhibit - this is broken as any client can unref all */
	ohm_plugin_conf_get_key (plugin, "suspend.fixme_inhibit", &value);
	if (value == 1) {
		g_warning ("not doing idle action as inhibited");
		return;
	}

	/* actually do the suspend */
	system_suspend (plugin);
}

/**
 * plugin_notify:
 * @plugin: This class instance
 *
 * Notify the plugin that a key marked with ohm_plugin_conf_interested ()
 * has it's value changed.
 * An enumerated numeric id rather than the key is returned for processing speed.
 */
static void
plugin_notify (OhmPlugin *plugin, gint id, gint value)
{
	if (id == CONF_BUTTON_LID_CHANGED) {
		if (value == 1) {
			lid_closed (plugin, TRUE);
		} else {
			lid_closed (plugin, FALSE);
		}
	} else if (id == CONF_BUTTON_POWER_CHANGED) {
		/* only match on button press, not release */
		if (value == 1) {
			power_button_pressed (plugin);
		}
	} else if (id == CONF_IDLE_POWERDOWN_CHANGED) {
		/* only match on idle, not reset */
		if (value == 1) {
			system_is_idle (plugin);
		}
	}
}

OHM_PLUGIN_DESCRIPTION (
	"suspend",			/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	NULL,				/* initialize */
	NULL,				/* destroy */
	plugin_notify			/* notify */
);

OHM_PLUGIN_SUGGESTS (
	"idle",
	"buttons"
);

OHM_PLUGIN_INTERESTED (
	{"button.power", CONF_BUTTON_POWER_CHANGED},
	{"button.lid", CONF_BUTTON_LID_CHANGED},
	{"idle.powerdown", CONF_IDLE_POWERDOWN_CHANGED},
	{"suspend.fixme_inhibit", CONF_INHIBIT_CHANGED}
);
