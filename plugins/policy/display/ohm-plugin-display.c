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
	CONF_TABLET_STATE_CHANGED,
	CONF_BRIGHTNESS_AC_CHANGED,
	CONF_BRIGHTNESS_BATTERY_CHANGED,
	CONF_BRIGHTNESS_IDLE_CHANGED,
	CONF_IDLE_POWERSAVE_CHANGED,
	CONF_IDLE_MOMENTARY_CHANGED,
	CONF_LAST
};

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
	ohm_plugin_suggest (plugin, "button");
	ohm_plugin_suggest (plugin, "backlight");
	return TRUE;
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
	ohm_plugin_conf_interested (plugin, "button.lid", CONF_LID_STATE_CHANGED);
	ohm_plugin_conf_interested (plugin, "button.tablet", CONF_TABLET_STATE_CHANGED);
	ohm_plugin_conf_interested (plugin, "idle.powersave", CONF_IDLE_POWERSAVE_CHANGED);
	ohm_plugin_conf_interested (plugin, "idle.momentary", CONF_IDLE_MOMENTARY_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_ac", CONF_BRIGHTNESS_AC_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_battery", CONF_BRIGHTNESS_BATTERY_CHANGED);
	ohm_plugin_conf_interested (plugin, "display.value_idle", CONF_BRIGHTNESS_IDLE_CHANGED);
}

static void
reset_brightness (OhmPlugin *plugin)
{
	gint onac;
	gint value;

	/* turn on dcon */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 1);

	ohm_plugin_conf_get_key (plugin, "acadapter.state", &onac);
	if (onac == TRUE) {
		ohm_plugin_conf_get_key (plugin, "display.value_ac", &value);
	} else {
		ohm_plugin_conf_get_key (plugin, "display.value_battery", &value);
	}

	/* dim screen to idle brightness */
	ohm_plugin_conf_set_key (plugin, "backlight.percent_brightness", value);
}

/* todo, we need to inhibit the screen from dimming */
static void
brightness_momentary (OhmPlugin *plugin, gboolean is_idle)
{
	gint lidshut;
	gint value;

	/* if lid shut */
	ohm_plugin_conf_get_key (plugin, "button.lid", &lidshut);
	if (lidshut == 1) {
		/* we've already turned off the screen */
		return;
	}

	/* if not idle any more */
	if (is_idle == FALSE) {
		reset_brightness (plugin);
		return;
	}

	/* dim screen to idle brightness */
	ohm_plugin_conf_get_key (plugin, "display.value_idle", &value);
	ohm_plugin_conf_set_key (plugin, "backlight.percent_brightness", value);
}

/* todo, we need to inhibit the screen from turning off */
static void
backlight_powersave (OhmPlugin *plugin, gboolean is_idle)
{
	gint lidshut;

	/* if lid shut */
	ohm_plugin_conf_get_key (plugin, "button.lid", &lidshut);
	if (lidshut == 1) {
		/* we've already turned off the screen */
		return;
	}

	/* if not idle any more */
	if (is_idle == FALSE) {
		reset_brightness (plugin);
		return;
	}

	/* turn off screen */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 0);
}

static void
lid_closed (OhmPlugin *plugin, gboolean is_closed)
{
	/* just turn on DCON */
	if (is_closed == FALSE) {
		ohm_plugin_conf_set_key (plugin, "backlight.state", 1);
		return;
	}
		
	/* just turn off dcon unconditionally */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 0);
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
	switch (id) {
	case CONF_BRIGHTNESS_AC_CHANGED:
	case CONF_BRIGHTNESS_BATTERY_CHANGED:
	case CONF_BRIGHTNESS_IDLE_CHANGED:
		reset_brightness (plugin);
		break;
	case CONF_IDLE_POWERSAVE_CHANGED:
		backlight_powersave (plugin, (value == 1));
		break;
	case CONF_LID_STATE_CHANGED:
		lid_closed (plugin, (value == 1));
		break;
	case CONF_TABLET_STATE_CHANGED:
		if (value == 0) {
			ohm_plugin_conf_set_key (plugin, "xrandr.position", 0);
		} else {
			ohm_plugin_conf_set_key (plugin, "xrandr.position", 1);
		}
		break;
	case CONF_AC_STATE_CHANGED:
		reset_brightness (plugin);
		break;
	case CONF_IDLE_MOMENTARY_CHANGED:
		brightness_momentary (plugin, (value == 1));
		break;
	}
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
