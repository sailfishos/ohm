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

enum {
	CONF_XORG_HASXAUTH_CHANGED,
	CONF_IDLE_TIMEOUT_CHANGED,
	CONF_IDLE_STATE_CHANGED,
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

	ohm_plugin_conf_get_key (plugin, "idle.timeout", &timeout);

	ohm_plugin_conf_set_key (plugin, "idle.state", 0);

	ret = idletime_alarm_set (idletime, 1, timeout);
	if (ret == FALSE) {
		g_error ("cannot set alarm");
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
	gint timeout;
	if (id == CONF_XORG_HASXAUTH_CHANGED) {
		if (value == 1) {
			plugin_connect_idletime (plugin);
		}
	} else if (id == CONF_IDLE_TIMEOUT_CHANGED ) {
		idletime_alarm_set (idletime, 1, value);
	} else if (id == CONF_IDLE_STATE_CHANGED ) {
		if (value == 0) {
			ohm_plugin_conf_get_key (plugin, "idle.timeout", &timeout);
			idletime_alarm_set (idletime, 1, timeout);
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

OHM_PLUGIN_PROVIDES ("idle.state");

OHM_PLUGIN_INTERESTED (
	{"xorg.has_xauthority", CONF_XORG_HASXAUTH_CHANGED},
	{"idle.timeout", CONF_IDLE_TIMEOUT_CHANGED},
	{"idle.state", CONF_IDLE_STATE_CHANGED});
