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
	CONF_IDLE_STATE_CHANGED,
	CONF_LAST
};


static void
init (OhmPlugin *plugin)
{
	gint first_timeout;

	ohm_plugin_conf_get_key (plugin, "timeouts.timer_momentary", &first_timeout);
	ohm_plugin_conf_set_key (plugin, "idle.timeout", first_timeout);
	ohm_plugin_conf_set_key (plugin, "idle.state", 0);
}

static void
notify (OhmPlugin *plugin, gint id, gint value)
{
	gint new_timeout;

	if (id != CONF_IDLE_STATE_CHANGED)
		return;

	switch (value) {
	case 0:
		ohm_plugin_conf_get_key (plugin, "timeouts.timer_momentary",
					 &new_timeout);
		ohm_plugin_conf_set_key (plugin, "idle.timeout", new_timeout);
		ohm_plugin_conf_set_key (plugin, "timeouts.momentary", 0);
		ohm_plugin_conf_set_key (plugin, "timeouts.powersave", 0);
		ohm_plugin_conf_set_key (plugin, "timeouts.powerdown", 0);
		break;
	case 1:
		ohm_plugin_conf_get_key (plugin, "timeouts.timer_powersave",
					 &new_timeout);
		ohm_plugin_conf_set_key (plugin, "idle.timeout", new_timeout);
		ohm_plugin_conf_set_key (plugin, "timeouts.momentary", 1);
		break;
	case 2:
		ohm_plugin_conf_get_key (plugin, "timeouts.timer_powerdown",
					 &new_timeout);
		ohm_plugin_conf_set_key (plugin, "idle.timeout", new_timeout);
		ohm_plugin_conf_set_key (plugin, "timeouts.powersave", 1);
		break;
	case 3:
		ohm_plugin_conf_set_key (plugin, "timeouts.powerdown", 1);
		break;
	default:
		break;
	}
}

OHM_PLUGIN_DESCRIPTION (
	"timeouts",			/* description */
	"0.0.1",			/* version */
	"rob.taylor@codethink.co.uk",	/* author */
	OHM_LICENSE_LGPL,		/* license */
	init,				/* initialize */
	NULL,				/* destroy */
	notify				/* notify */
);

OHM_PLUGIN_PROVIDES (
	"timeouts.momentary",
	"timeouts.powersave",
	"timeouts.powerdown"
);

OHM_PLUGIN_REQUIRES (
	"idle"
);

OHM_PLUGIN_INTERESTED (
	{"idle.state", CONF_IDLE_STATE_CHANGED}
);
