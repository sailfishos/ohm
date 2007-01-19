/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
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
	CONF_BACKLIGHTCHANGED,
	CONF_ACCHANGED,
	CONF_LAST
};

/**
 * plugin_load:
 * @plugin: This class instance
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static void
plugin_load (OhmPlugin *plugin)
{
	g_debug ("plug:load plugin %s", ohm_plugin_get_name (plugin));

	/* add in the required, suggested and prevented plugins */
//	ohm_plugin_require (plugin, "libmoo.so");
	ohm_plugin_suggest (plugin, "libtemperature.so");
	ohm_plugin_prevent (plugin, "libembedded.so");

	/* tell ohmd what keys we are going to provide - don't set keys
	 * unless you provide them or you know a plugin you require provides them */
	ohm_plugin_conf_provide (plugin, "battery.percentage");
}

/**
 * plugin_unload:
 * @plugin: This class instance
 *
 * Called just beforet the plugin module is unloaded, and gives the plugin
 * a chance to free private memory.
 */
static void
plugin_unload (OhmPlugin *plugin)
{
	g_debug ("plug:unload plugin");
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
	g_debug ("plug:coldplug");
	gint value;
	ohm_plugin_conf_set_key (plugin, "battery.percentage", 99);
	ohm_plugin_conf_get_key (plugin, "battery.percentage", &value);

	/* these don't have to be one enum per key, you can clump them as classes */
	ohm_plugin_conf_interested (plugin, "backlight.value_foo", CONF_BACKLIGHTCHANGED);
	ohm_plugin_conf_interested (plugin, "system.ac_state", CONF_ACCHANGED);

	g_debug ("plug:got conf from plugin! %i", value);	
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
	g_debug ("plug:plugin_conf_notify %i: %i", id, value);
	/* using an integer enumeration is much faster than a load of strcmp's */
	if (id == CONF_BACKLIGHTCHANGED) {
		g_error ("plug:backlight changed, so maybe we need to update something or re-evaluate policy");
	} else if (id == CONF_ACCHANGED) {
		g_error ("plug:ac status changed, so maybe we need to update something or re-evaluate policy");
	}
}

static OhmPluginInfo plugin_info = {
	"OHM HAL Battery",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_load,			/* load */
	plugin_unload,			/* unload */
	plugin_coldplug,		/* coldplug */
	plugin_conf_notify,		/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
