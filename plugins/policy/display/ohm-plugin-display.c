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

enum {
	CONF_AC_STATE_CHANGED,
	CONF_LID_STATE_CHANGED,
	CONF_SYSTEM_IDLE_CHANGED,
	CONF_BRIGHTNESS_AC_CHANGED,
	CONF_BRIGHTNESS_BATTERY_CHANGED,
	CONF_BRIGHTNESS_IDLE_CHANGED,
	CONF_TIME_IDLE_CHANGED,
	CONF_TIME_OFF_CHANGED,
	CONF_LAST
};

typedef struct {
	gint ac_state;
	gint lid_state;
	gint system_idle;
	gint state;
	gint brightness;
	gint brightness_ac;
	gint brightness_battery;
	gint brightness_idle;
	gint time_idle;
	gint time_off;
	gint levels;
} OhmPluginCacheData;

static OhmPluginCacheData data;


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
	/* FIXME: detect if we have any backlights in the system and return false if not */
	/* add in the required, suggested and prevented plugins */
	ohm_plugin_suggest (plugin, "idle");
	ohm_plugin_suggest (plugin, "acadapter");
	ohm_plugin_suggest (plugin, "lid");
	ohm_plugin_suggest (plugin, "backlight");
	return TRUE;
}

/**
 * check_system_idle_state:
 * @plugin: This class instance
 *
 * Check the idle times, and dim the backlight if required
 */
static void
check_system_backlight_state (OhmPlugin *plugin)
{
	/* lid closed, just turn off display */
	if (data.lid_state == 1) {
		/* turn off screen unconditionally */
		data.state = 0;
		data.brightness = 0;
	} else if (data.system_idle > data.time_off) {
		/* turn off screen after 20 seconds */
		data.state = 0;
		data.brightness = 0;
	} else if (data.system_idle > data.time_idle) {
		/* dim screen after 5 seconds */
		data.state = 1;
		data.brightness = data.brightness_idle;
	} else {
		/* set brightness to default value */
		data.state = 1;
		if (data.ac_state == 1) {
			data.brightness = data.brightness_ac;
		} else {
			data.brightness = data.brightness_battery;
		}
	}
//	ohm_plugin_conf_set_key (plugin, "backlight.state", data.state);
	ohm_plugin_conf_set_key (plugin, "backlight.percent_brightness", data.brightness);
	g_debug ("setting state %i and brightness %i", data.state, data.brightness);
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
	/* interested keys */
	ohm_plugin_conf_interested (plugin, "acadapter.state", CONF_AC_STATE_CHANGED);
	ohm_plugin_conf_interested (plugin, "lid.state", CONF_LID_STATE_CHANGED);
	ohm_plugin_conf_interested (plugin, "idle.system_idle", CONF_SYSTEM_IDLE_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_ac", CONF_BRIGHTNESS_AC_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_battery", CONF_BRIGHTNESS_BATTERY_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_idle", CONF_BRIGHTNESS_IDLE_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.time_idle", CONF_TIME_IDLE_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.time_off", CONF_TIME_OFF_CHANGED);

	/* preference values */
	ohm_plugin_conf_get_key (plugin, "acadapter.state", &(data.ac_state));
	ohm_plugin_conf_get_key (plugin, "lid.state", &(data.lid_state));
	ohm_plugin_conf_get_key (plugin, "idle.system_idle", &(data.system_idle));
	ohm_plugin_conf_get_key (plugin, "display.value_ac", &(data.brightness_ac));
	ohm_plugin_conf_get_key (plugin, "display.value_battery", &(data.brightness_battery));
	ohm_plugin_conf_get_key (plugin, "display.value_idle", &(data.brightness_idle));
	ohm_plugin_conf_get_key (plugin, "display.time_idle", &(data.time_idle));
	ohm_plugin_conf_get_key (plugin, "display.time_off", &(data.time_off));

	check_system_backlight_state (plugin);
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
	if (id == CONF_SYSTEM_IDLE_CHANGED) {
		data.system_idle = value;
	} else if (id == CONF_LID_STATE_CHANGED) {
		data.lid_state = value;
	} else if (id == CONF_AC_STATE_CHANGED) {
		data.ac_state = value;
	} else if (id == CONF_BRIGHTNESS_AC_CHANGED) {
		data.brightness_ac = value;
	} else if (id == CONF_BRIGHTNESS_BATTERY_CHANGED) {
		data.brightness_battery = value;
	} else if (id == CONF_BRIGHTNESS_IDLE_CHANGED) {
		data.brightness_idle = value;
	} else if (id == CONF_TIME_IDLE_CHANGED) {
		data.time_idle = value;
	} else if (id == CONF_TIME_OFF_CHANGED) {
		data.time_off = value;
	}
	check_system_backlight_state (plugin);
}

static OhmPluginInfo plugin_info = {
	"Display",			/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	NULL,				/* unload */
	plugin_coldplug,		/* coldplug */
	plugin_conf_notify,		/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);