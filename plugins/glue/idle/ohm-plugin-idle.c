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
#include <libidletime.h>
#include <stdlib.h>

static LibIdletime *idletime = NULL;
static gint64 timeout_offset = 0;

enum {
	CONF_XORG_HASXAUTH_CHANGED,
	CONF_IDLE_TIMEOUT_CHANGED,
	CONF_IDLE_RESET,
	CONF_LAST
};

static void
ohm_alarm_expired_cb (LibIdletime *idletime, guint alarm, gpointer data)
{
	OhmPlugin *plugin = (OhmPlugin *) data;
	gint state;

	if (alarm == 0 ) {
		/* activity, reset state to 0 */
		ohm_plugin_conf_set_key (plugin, "idle.state", 0);
		timeout_offset = 0;
	} else {
		ohm_plugin_conf_get_key (plugin, "idle.state", &state);
		ohm_plugin_conf_set_key (plugin, "idle.state", state+1);
	}
	g_print ("[evt %i]\n", alarm);
}

static void
plugin_connect_idletime (OhmPlugin *plugin)
{
	gboolean ret;
	gint timeout;
	const gchar *xauth;

	xauth = getenv ("XAUTHORITY");
	g_debug ("connecting with %s", xauth);

	idletime = idletime_new ();
	if (idletime == NULL) {
		g_error ("cannot create idletime");
	}
	g_signal_connect (idletime, "alarm-expired",
			  G_CALLBACK (ohm_alarm_expired_cb), plugin);

	ohm_plugin_conf_set_key (plugin, "idle.state", 0);
	ohm_plugin_conf_set_key (plugin, "idle.reset", 0);

	ohm_plugin_conf_get_key (plugin, "idle.timeout", &timeout);

	if (timeout != 0) {
		ret = idletime_alarm_set (idletime, 1, timeout);
		if (ret == FALSE) {
			g_error ("cannot set alarm");
		}
	}
}

/**
 * plugin_initalize:
 * @plugin: This class instance
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_initalize (OhmPlugin *plugin)
{
	gint value;

	/* check system inhibit - this is broken as any client can unref all */
	ohm_plugin_conf_get_key (plugin, "xorg.has_xauthority", &value);
	if (value == 1) {
		plugin_connect_idletime (plugin);
	}

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
	g_debug ("idle got notify, id = %d, value = %d", id, value);
	gint timeout;
	if (id == CONF_XORG_HASXAUTH_CHANGED) {
		if (value == 1) {
			plugin_connect_idletime (plugin);
		}
	}
	if (idletime) {
		if (id == CONF_IDLE_TIMEOUT_CHANGED ) {
			g_debug("setting new timeout %d", value);
			idletime_alarm_set (idletime, 1, timeout_offset+value);
		} else if (id == CONF_IDLE_RESET && value == 1) {
			timeout_offset = idletime_get_current_idle (idletime);
			ohm_plugin_conf_get_key (plugin, "idle.timeout", &timeout);
			g_debug ("idle plugin reset. current timeout = %d, timeout-offset = %lld", timeout, timeout_offset);

			/* most of the time this isn't needed as the below set
			 * of idle.state will result in a timout change,
			 * but just incase it doesn't happen. or the
			 * new timeout is the same as the old..
			 */
			idletime_alarm_set (idletime, 1, timeout_offset+timeout);
			ohm_plugin_conf_set_key (plugin, "idle.reset", 0);
			ohm_plugin_conf_set_key (plugin, "idle.state", 0);
		}
	}
}
static void
plugin_destroy (OhmPlugin *plugin)
{
	if (idletime)
		g_object_unref (idletime);
}

OHM_PLUGIN_DESCRIPTION (
	"OHM IdleTime",			/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initalize,		/* initalize */
	plugin_destroy,			/* destroy */
	plugin_notify		/* notify */
);

OHM_PLUGIN_REQUIRES ("xorg");

OHM_PLUGIN_PROVIDES ("idle.state", "idle.timeout", "idle.reset");

OHM_PLUGIN_INTERESTED (
	{"xorg.has_xauthority", CONF_XORG_HASXAUTH_CHANGED},
	{"idle.timeout", CONF_IDLE_TIMEOUT_CHANGED},
	{"idle.reset", CONF_IDLE_RESET});
