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
#include <ohm-plugin.h>

enum {
	CONF_XRANDR_POSITION_CHANGED,
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
	ohm_plugin_require (plugin, "xorg");
	ohm_plugin_conf_provide (plugin, "xrandr.position");
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
	ohm_plugin_conf_set_key (plugin, "xrandr.position", 0);
	ohm_plugin_conf_interested (plugin, "xrandr.position", CONF_XRANDR_POSITION_CHANGED);
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
	if (id == CONF_XRANDR_POSITION_CHANGED) {
		if (value == 0) {
			ohm_plugin_spawn_async (plugin, "/usr/bin/xrandr -o left");
		} else {
			ohm_plugin_spawn_async (plugin, "/usr/bin/xrandr -o normal");
		}
	}
}

static OhmPluginInfo plugin_info = {
	"OHM HAL XRANDR",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	NULL,				/* unload */
	plugin_coldplug,		/* coldplug */
	plugin_conf_notify,		/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
