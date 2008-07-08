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
#include <string.h>
#include <ohm/ohm-plugin.h>

enum {
	CONF_XRANDR_POSITION_CHANGED,
	CONF_LAST
};

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
	ohm_plugin_conf_set_key (plugin, "xrandr.position", 0);
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
	if (id == CONF_XRANDR_POSITION_CHANGED) {
		if (value == 0) {
			ohm_plugin_spawn_async (plugin, "/usr/bin/xrandr -o normal");
		} else {
			ohm_plugin_spawn_async (plugin, "/usr/bin/xrandr -o left");
		}
	}
}

OHM_PLUGIN_DESCRIPTION (
	"OHM HAL XRANDR",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initialize,		/* initialize */
	NULL,				/* destroy */
	plugin_notify			/* notify */
);

OHM_PLUGIN_REQUIRES ("xorg");

OHM_PLUGIN_PROVIDES ("xrandr.position");

OHM_PLUGIN_INTERESTED ({"xrandr.position", CONF_XRANDR_POSITION_CHANGED});
