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

#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include "ohm-plugin.h"

static void ohm_plugin_destroy (OhmPlugin * plugin);
static OhmPlugin *ohm_plugins_find_with_name (const gchar * name);
static GList *plugins = NULL;
	
#define LIBDIR ".libs"

G_MODULE_EXPORT void
ohm_plugin_get_key (int value)
{
	/* do bar things */
	g_debug ("server! %i", value);
}

OhmPlugin *
ohm_plugin_load (const gchar *name)
{
	OhmPlugin *plugin;
	gchar *path;
	GModule *handle;

	gboolean (*ohm_init_plugin) (OhmPlugin *);

	g_return_val_if_fail (name != NULL, NULL);

	plugin = ohm_plugins_find_with_name (name);
	if (plugin) {
		++plugin->ref_count;
		return plugin;
	}

	path = g_build_filename (LIBDIR, name, NULL);
	handle = g_module_open (path, 0);
	if (!handle) {
		g_error ("opening module %s failed : %s\n", path, g_module_error ());
	}
	g_free (path);

	if (!g_module_symbol (handle, "ohm_init_plugin", (gpointer) & ohm_init_plugin)) {
		g_module_close (handle);
		g_error ("could not find init function in plugin\n");
	}

	plugin = g_new0 (OhmPlugin, 1);
	plugin->info = NULL;
	plugin->handle = handle;
	plugin->name = g_strdup (name);
	plugin->ref_count = 1;
	plugins = g_list_append (plugins, plugin);

	if (!ohm_init_plugin (plugin) || plugin->info == NULL) {
		g_print ("init error or no info defined");
		ohm_plugin_destroy (plugin);
		return NULL;
	}

	if (plugin->info->load != NULL)
		plugin->info->load (plugin);

	return plugin;
}

gboolean
ohm_plugin_unload (OhmPlugin * plugin)
{
	g_return_val_if_fail (plugin != NULL, FALSE);
	g_return_val_if_fail (plugin->ref_count > 0, FALSE);

	if (--plugin->ref_count > 0)
		return TRUE;

	if (plugin->info->unload != NULL)
		plugin->info->unload (plugin);

	ohm_plugin_destroy (plugin);

	return TRUE;
}

static void
ohm_plugin_destroy (OhmPlugin * plugin)
{
	g_return_if_fail (plugin != NULL);

	plugins = g_list_remove (plugins, plugin);

	if (plugin->handle != NULL)
		g_module_close (plugin->handle);

	if (plugin->name != NULL)
		g_free (plugin->name);

	g_free (plugin);
}

static OhmPlugin *
ohm_plugins_find_with_name (const gchar * name)
{
	OhmPlugin *plugin;
	GList *l;

	for (l = plugins; l != NULL; l = l->next) {
		plugin = l->data;
		if (plugin->name != NULL && !strcmp (plugin->name, name))
			return plugin;
	}

	return NULL;
}

G_CONST_RETURN gchar *
ohm_plugin_get_name (OhmPlugin * plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->name;
}

G_CONST_RETURN gchar *
ohm_plugin_get_version (OhmPlugin * plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->info->version;
}

G_CONST_RETURN gchar *
ohm_plugin_get_author (OhmPlugin * plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->info->author;
}
#if 0

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	g_type_init ();

OhmPlugin *plugin;
plugin = ohm_plugin_load ("libpluginbattery.so");
ohm_plugin_unload (plugin);

	return 0;
}
#endif
