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

static void
hal_condition_cb (OhmPlugin   *plugin,
		  guint        id,
		  const gchar *name,
		  const gchar *detail)
{
	gboolean state;
	if (strcmp (name, "ButtonPressed") == 0) {
		if (strcmp (detail, "power") == 0) {
			ohm_plugin_conf_set_key (plugin, "button.power", 1);
			ohm_plugin_conf_set_key (plugin, "button.power", 0);
		}
		if (strcmp (detail, "lid") == 0) {
			ohm_plugin_hal_get_bool (plugin, id, "button.state.value", &state);
			ohm_plugin_conf_set_key (plugin, "button.lid", state);
		}
		if (strcmp (detail, "tablet_mode") == 0) {
			ohm_plugin_hal_get_bool (plugin, id, "button.state.value", &state);
			ohm_plugin_conf_set_key (plugin, "button.tablet", state);
		}
	}
}

/**
 * plugin_initalize:
 * @plugin: This class instance
 *
 * Read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_initalize (OhmPlugin *plugin)
{
	/* initalize HAL */
	ohm_plugin_hal_init (plugin);

	/* we want this function to get the property modified events for all devices */
	ohm_plugin_hal_use_condition (plugin, hal_condition_cb);

	/* get the only device with capability and watch it */
	ohm_plugin_hal_add_device_capability (plugin, "button");

	/* fixme: assumes open on boot */
	ohm_plugin_conf_set_key (plugin, "button.lid", 0);
	ohm_plugin_conf_set_key (plugin, "button.power", 0);
	ohm_plugin_conf_set_key (plugin, "button.tablet", 0);
}


OHM_PLUGIN_DESCRIPTION (
	"OHM HAL Buttons",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initalize,		/* initalize */
	NULL,				/* destroy */
	NULL				/* notify */
);

OHM_PLUGIN_PROVIDES (
	"button.lid",
	"button.power",
	"button.tablet"
);
