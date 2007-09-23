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
	CONF_BACKLIGHT_STATE_CHANGED,
	CONF_LID_STATE_CHANGED,
	CONF_TABLET_STATE_CHANGED,
	CONF_BRIGHTNESS_AC_CHANGED,
	CONF_BRIGHTNESS_BATTERY_CHANGED,
	CONF_BRIGHTNESS_IDLE_CHANGED,
	CONF_TIMEOUTS_POWERSAVE_CHANGED,
	CONF_TIMEOUTS_MOMENTARY_CHANGED,
	CONF_LAST
};

static void
reset_brightness (OhmPlugin *plugin)
{
	gint onac;
	gint value;

	/* FIXME: turn on dcon -- why was this here? */
	/* ohm_plugin_conf_set_key (plugin, "backlight.state", 1); */

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
	gint state;

	ohm_plugin_conf_get_key (plugin, "backlight.state", &state);
	if (state == 0) {
		/* work round a idletime bugs */
		return;
	}

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
	gint state;

	ohm_plugin_conf_get_key (plugin, "backlight.state", &state);
	if (is_idle && state == 0) {
		/* work round a idletime bugs */
		return;
	}

	/* if lid shut */
	ohm_plugin_conf_get_key (plugin, "button.lid", &lidshut);
	if (lidshut == 1) {
		/* we've already turned off the screen */
		return;
	}

	/* if not idle any more */
	if (is_idle == FALSE) {
		ohm_plugin_conf_set_key (plugin, "backlight.state", 1);
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
	switch (id) {
	case CONF_BRIGHTNESS_AC_CHANGED:
	case CONF_BRIGHTNESS_BATTERY_CHANGED:
	case CONF_BRIGHTNESS_IDLE_CHANGED:
		reset_brightness (plugin);
		break;
	case CONF_TIMEOUTS_POWERSAVE_CHANGED:
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
	case CONF_BACKLIGHT_STATE_CHANGED:
		reset_brightness (plugin);
		break;
	case CONF_TIMEOUTS_MOMENTARY_CHANGED:
		brightness_momentary (plugin, (value == 1));
		break;
	}
}

OHM_PLUGIN_DESCRIPTION (
	"Display",			/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	NULL,				/* initialize */
	NULL,				/* destroy */
	plugin_notify			/* notify */
);

OHM_PLUGIN_SUGGESTS (
	"timeouts",
	"acadapter",
	"buttons",
	"xrandr",
	"backlight"
);

OHM_PLUGIN_INTERESTED (
	{"acadapter.state", CONF_AC_STATE_CHANGED},
	{"backlight.state", CONF_BACKLIGHT_STATE_CHANGED},
	{"button.lid", CONF_LID_STATE_CHANGED},
	{"button.tablet", CONF_TABLET_STATE_CHANGED},
	{"timeouts.powersave", CONF_TIMEOUTS_POWERSAVE_CHANGED},
	{"timeouts.momentary", CONF_TIMEOUTS_MOMENTARY_CHANGED},
	{"display.value_ac", CONF_BRIGHTNESS_AC_CHANGED},
	{"display.value_battery", CONF_BRIGHTNESS_BATTERY_CHANGED},
	{"display.value_idle", CONF_BRIGHTNESS_IDLE_CHANGED}
);
