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

#ifndef SERVER_H
#define SERVER_H

#include <glib-object.h>

void	ohm_plugin_get_key (int value);

typedef struct _OhmPlugin OhmPlugin;
typedef struct _OhmPluginInfo OhmPluginInfo;

struct _OhmPluginInfo {
	gchar *description;
	gchar *version;
	gchar *author;
	void (*load) (OhmPlugin * plugin);
	void (*unload) (OhmPlugin * plugin);
};

struct _OhmPlugin {
	OhmPluginInfo *info;
	GType type;
	GModule *handle;
	gchar *name;
	guint ref_count;
};

#define OHM_INIT_PLUGIN(initfunc, plugininfo) \
G_MODULE_EXPORT gboolean ohm_init_plugin(OhmPlugin *plugin) { \
				plugin->info = &(plugininfo); \
				return initfunc((plugin)); \
}

OhmPlugin *ohm_plugin_load (const gchar * name);
gboolean ohm_plugin_unload (OhmPlugin * plugin);

G_CONST_RETURN gchar *ohm_plugin_get_name (OhmPlugin * plugin);
G_CONST_RETURN gchar *ohm_plugin_get_version (OhmPlugin * plugin);
G_CONST_RETURN gchar *ohm_plugin_get_author (OhmPlugin * plugin);

#endif /* SERVER_H */
