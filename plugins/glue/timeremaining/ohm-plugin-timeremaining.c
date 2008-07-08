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

#include <ohm/ohm-plugin.h>

enum {
	CONF_BATTERY_PERCENT_CHANGED,
	CONF_AC_STATE_CHANGED,
	CONF_LAST
};

typedef struct {
	gint percentage;
	gint ac_state;
} OhmPluginCacheData;

static OhmPluginCacheData data;

/**
 * check_system_power_state:
 * @plugin: This class instance
 *
 * Check the battery, and set the low and critical values if battery is low
 */
static void
check_system_power_state (OhmPlugin *plugin)
{
	ohm_plugin_conf_set_key (plugin, "timeremaining.to_charge", 10);
	ohm_plugin_conf_set_key (plugin, "timeremaining.to_discharge", 10);
}

/**
 * plugin_initialize:
 * @plugin: This class instance
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_initialize (OhmPlugin *plugin)
{
	/* initial values */
	ohm_plugin_conf_get_key (plugin, "battery.percentage", &(data.percentage));
	ohm_plugin_conf_get_key (plugin, "acadapter.state", &(data.ac_state));

	check_system_power_state (plugin);
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
	if (id == CONF_BATTERY_PERCENT_CHANGED) {
		data.percentage = value;
		check_system_power_state (plugin);
	} else if (id == CONF_AC_STATE_CHANGED) {
		data.ac_state = value;
		check_system_power_state (plugin);
	}
}

OHM_PLUGIN_DESCRIPTION (
	"OHM timeremaining",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initialize,		/* initialize */
	NULL,				/* destroy */
	plugin_notify			/* notify */
);

OHM_PLUGIN_SUGGESTS ("battery", "acadapter");

OHM_PLUGIN_PROVIDES ("timeremaining.to_charge", "timeremaining.to_discharge");

OHM_PLUGIN_INTERESTED (
	{"battery.percentage", CONF_BATTERY_PERCENT_CHANGED},
	{"acadapter.state", CONF_AC_STATE_CHANGED}
);
	
