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

#include "../ohmd/ohm-plugin.h"

static gboolean 
init_plugin (OhmPlugin *plugin)
{
	g_debug ("plugin init");
	return TRUE;
}

static void
load_plugin (OhmPlugin *plugin)
{
	g_debug ("load plugin");
	ohm_plugin_get_key (888);
}

static void
unload_plugin (OhmPlugin *plugin)
{
	g_debug ("unload plugin");
}

static OhmPluginInfo plugin_info = {
	"Ohm RTP plugin",                              /* description */
	"0.1.0",                                            /* version */
	"richard@hughsie.com",                              /* author */
	load_plugin,                                        /* load */
	unload_plugin,                                      /* unload */
};

OHM_INIT_PLUGIN (init_plugin, plugin_info);
