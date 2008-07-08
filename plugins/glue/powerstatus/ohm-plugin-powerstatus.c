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
	CONF_PERCENT_LOW_CHANGED,
	CONF_PERCENT_CRITICAL_CHANGED,
	CONF_BATTERY_CHANGED,
	CONF_LAST
};

typedef struct {
	gint percentage;
	gint percentage_low;
	gint percentage_critical;
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
	gint is_low;
	gint is_critical;

	if (data.percentage < data.percentage_critical) {
		is_low = 1;
		is_critical = 1;
	} else if (data.percentage < data.percentage_low) {
		is_low = 1;
		is_critical = 0;
	} else {
		is_low = 0;
		is_critical = 0;
	}
	ohm_plugin_conf_set_key (plugin, "powerstatus.low", is_low);
	ohm_plugin_conf_set_key (plugin, "powerstatus.critical", is_critical);
}

/**
 * plugin_initialise:
 * @plugin: This class instance
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_initialise (OhmPlugin *plugin)
{
	/* initial values */
	ohm_plugin_conf_get_key (plugin, "battery.percentage", &(data.percentage));
	ohm_plugin_conf_get_key (plugin, "powerstatus.percentage_low", &(data.percentage_low));
	ohm_plugin_conf_get_key (plugin, "powerstatus.percentage_critical", &(data.percentage_critical));

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
	if (id == CONF_BATTERY_CHANGED) {
		data.percentage = value;
		check_system_power_state (plugin);
	} else if (id == CONF_PERCENT_LOW_CHANGED) {
		data.percentage_low = value;
		check_system_power_state (plugin);
	} else if (id == CONF_PERCENT_CRITICAL_CHANGED) {
		data.percentage_critical = value;
		check_system_power_state (plugin);
	}
}

OHM_PLUGIN_DESCRIPTION (
	"OHM PowerStatus",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initialise,		/* initialise */
	NULL,				/* destroy */
	plugin_notify			/* notify */
);

OHM_PLUGIN_SUGGESTS ("battery");

OHM_PLUGIN_PROVIDES ("powerstatus.low", "powerstatus.critical");

OHM_PLUGIN_INTERESTED (
	{"battery.percentage", CONF_BATTERY_CHANGED},
	{"powerstatus.percentage_low", CONF_BATTERY_CHANGED},
	{"powerstatus.percentage_critical", CONF_BATTERY_CHANGED}
);


