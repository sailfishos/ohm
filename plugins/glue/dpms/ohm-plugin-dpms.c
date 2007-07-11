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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <gmodule.h>
#include <glib.h>

#include <ohm-plugin.h>

#include <X11/Xutil.h>
#include <X11/Xproto.h>			/* for CARD16 */
#include <X11/extensions/dpms.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpmsstr.h>

static Display *dpy;

enum {
	CONF_BACKLIGHT_STATE_CHANGED,
	CONF_LAST
};

typedef enum {
	OHM_DPMS_MODE_ON,
	OHM_DPMS_MODE_STANDBY,
	OHM_DPMS_MODE_SUSPEND,
	OHM_DPMS_MODE_OFF
} OhmDpmsMode;

/**
 * plugin_preload:
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static gboolean
plugin_preload (OhmPlugin *plugin)
{
	/* add in the required, suggested and prevented plugins */
	ohm_plugin_require (plugin, "backlight");
	return TRUE;
}


/**
 * ohm_dpms_get_mode:
 */
static gboolean
ohm_dpms_get_mode (OhmDpmsMode *mode)
{
	OhmDpmsMode result;
	int event_number;
	int error_number;
	BOOL enabled = FALSE;
	CARD16 state;

	if (! DPMSQueryExtension (dpy, &event_number, &error_number)) {
		/* Server doesn't know -- assume the monitor is on. */
		result = OHM_DPMS_MODE_ON;

	} else if (! DPMSCapable (dpy)) {
		/* Server says the monitor doesn't do power management -- so it's on. */
		result = OHM_DPMS_MODE_ON;

	} else {
		DPMSInfo (dpy, &state, &enabled);
		if (! enabled) {
			/* Server says DPMS is disabled -- so the monitor is on. */
			result = OHM_DPMS_MODE_ON;
		} else {
			switch (state) {
			case DPMSModeOn:
				result = OHM_DPMS_MODE_ON;
				break;
			case DPMSModeStandby:
				result = OHM_DPMS_MODE_STANDBY;
				break;
			case DPMSModeSuspend:
				result = OHM_DPMS_MODE_SUSPEND;
				break;
			case DPMSModeOff:
				result = OHM_DPMS_MODE_OFF;
				break;
			default:
				result = OHM_DPMS_MODE_ON;
				break;
			}
		}
	}

	if (mode) {
		*mode = result;
	}
	return TRUE;
}

/**
 * ohm_dpms_set_mode:
 */
static gboolean
ohm_dpms_set_mode (OhmDpmsMode mode)
{
	CARD16 state;
	OhmDpmsMode current_mode;

	if (dpy == NULL) {
		g_debug ("cannot open display");
		return FALSE;
	}

	if (! DPMSCapable (dpy)) {
		g_debug ("display not DPMS capable");
		return FALSE;
	}

	if (mode == OHM_DPMS_MODE_ON) {
		state = DPMSModeOn;
	} else if (mode == OHM_DPMS_MODE_STANDBY) {
		state = DPMSModeStandby;
	} else if (mode == OHM_DPMS_MODE_SUSPEND) {
		state = DPMSModeSuspend;
	} else if (mode == OHM_DPMS_MODE_OFF) {
		state = DPMSModeOff;
	} else {
		g_warning ("invalid state");
		state = DPMSModeOn;
	}

	/* make sure we are not trying to set the current screen mode, else we flicker */
	ohm_dpms_get_mode (&current_mode);
	if (current_mode != mode) {
		g_debug ("Setting DPMS state");
		if (! DPMSForceLevel (dpy, state)) {
			g_warning ("Could not change DPMS mode");
			return FALSE;
		}
	}

	XSync (dpy, FALSE);
	return TRUE;
}

/**
 * plugin_coldplug:
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_coldplug (OhmPlugin *plugin)
{
	/* we can assume DPMS is on */
	ohm_plugin_conf_set_key (plugin, "backlight.state", 1);

	/* interested keys */
	ohm_plugin_conf_interested (plugin, "backlight.state", CONF_BACKLIGHT_STATE_CHANGED);

	/* open display, need to free using XCloseDisplay */
	dpy = XOpenDisplay (":0"); /* fixme: don't assume :0 */
}

/**
 * plugin_unload:
 *
 * Unload drivers, free memory.
 */
static void
plugin_unload (OhmPlugin *plugin)
{
	XCloseDisplay (dpy);
	dpy = NULL;
}

/**
 * plugin_conf_notify:
 *
 * Notify the plugin that a key marked with ohm_plugin_conf_interested ()
 * has it's value changed.
 * An enumerated numeric id rather than the key is returned for processing speed.
 */
static void
plugin_conf_notify (OhmPlugin *plugin, gint id, gint value)
{
	if (id == CONF_BACKLIGHT_STATE_CHANGED) {
		if (value == 0) {
			ohm_dpms_set_mode (OHM_DPMS_MODE_OFF);
		} else {
			ohm_dpms_set_mode (OHM_DPMS_MODE_ON);
		}
	}
}

static OhmPluginInfo plugin_info = {
	"OHM DPMS",			/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	plugin_unload,			/* unload */
	plugin_coldplug,		/* coldplug */
	plugin_conf_notify,		/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
