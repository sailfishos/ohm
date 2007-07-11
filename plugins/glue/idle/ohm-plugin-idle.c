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
#include <gdk/gdk.h>

static LibIdletime *idletime;

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
	/* tell ohmd what keys we are going to provide */
	ohm_plugin_conf_provide (plugin, "idle.momentary");
	ohm_plugin_conf_provide (plugin, "idle.powersave");
	ohm_plugin_conf_provide (plugin, "idle.powerdown");
	return TRUE;
}

static void
ohm_alarm_expired_cb (LibIdletime *idletime, guint alarm, gpointer data)
{
	OhmPlugin *plugin = (OhmPlugin *) data;
	if (alarm == 0) {
		ohm_plugin_conf_set_key (plugin, "idle.momentary", 0);
		ohm_plugin_conf_set_key (plugin, "idle.powersave", 0);
		ohm_plugin_conf_set_key (plugin, "idle.powerdown", 0);
	} else if (alarm == 1) {
		ohm_plugin_conf_set_key (plugin, "idle.momentary", 1);
	} else if (alarm == 2) {
		ohm_plugin_conf_set_key (plugin, "idle.powersave", 1);
	} else if (alarm == 3) {
		ohm_plugin_conf_set_key (plugin, "idle.powerdown", 1);
	}
	g_print ("[evt %i]\n", alarm);
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
	gint momentary;
	gint powersave;
	gint powerdown;

	gdk_init (NULL, NULL);
	idletime = idletime_new ();
	if (idletime == NULL) {
		g_error ("cannot create idletime");
	}
	g_signal_connect (idletime, "alarm-expired",
			  G_CALLBACK (ohm_alarm_expired_cb), plugin);

	ohm_plugin_conf_get_key (plugin, "idle.timer_momentary", &momentary);
	ohm_plugin_conf_get_key (plugin, "idle.timer_powersave", &powersave);
	ohm_plugin_conf_get_key (plugin, "idle.timer_powerdown", &powerdown);

	ret = idletime_alarm_set (idletime, 1, momentary);
	if (ret == FALSE) {
		g_error ("cannot set alarm 1");
	}
	ret = idletime_alarm_set (idletime, 2, powersave);
	if (ret == FALSE) {
		g_error ("cannot set alarm 2");
	}
	ret = idletime_alarm_set (idletime, 3, powerdown);
	if (ret == FALSE) {
		g_error ("cannot set alarm 3");
	}
}

static void
plugin_unload (OhmPlugin *plugin)
{
	g_object_unref (idletime);
}

static OhmPluginInfo plugin_info = {
	"OHM IdleTime",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	plugin_unload,			/* unload */
	plugin_coldplug,		/* coldplug */
	NULL,				/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
