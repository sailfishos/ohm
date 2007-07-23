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

/* Provides the bridge between the .so plugin and intraprocess communication */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <glib/gi18n.h>
#include <gmodule.h>
#include <libhal.h>

#include "ohm-debug.h"
#include "ohm-plugin.h"
#include "ohm-conf.h"
#include "ohm-marshal.h"

#define OHM_PLUGIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_PLUGIN, OhmPluginPrivate))

struct OhmPluginPrivate
{
	OhmConf			*conf;
	OhmPluginInfo		*info;
	GModule			*handle;
	gchar			*name;
	/* not assigned unless a plugin uses hal */
	LibHalContext		*hal_ctx;
	GPtrArray		*hal_udis;
	OhmPluginHalPropMod	 hal_property_changed_cb;
	OhmPluginHalCondition	 hal_condition_cb;
};

enum {
	ADD_INTERESTED,
	ADD_REQUIRE,
	ADD_SUGGEST,
	ADD_PREVENT,
	LAST_SIGNAL
};

static guint	     signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (OhmPlugin, ohm_plugin, G_TYPE_OBJECT)

/**
 * ohm_plugin_get_key:
 **/
gboolean
ohm_plugin_require (OhmPlugin   *plugin,
		    const gchar *name)
{
	ohm_debug ("emitting add-require '%s'", name);
	g_signal_emit (plugin, signals[ADD_REQUIRE], 0, name);
	return TRUE;
}

/**
 * ohm_plugin_add_notify_key:
 **/
gboolean
ohm_plugin_suggest (OhmPlugin   *plugin,
		    const gchar *name)
{
	ohm_debug ("emitting add-suggest '%s'", name);
	g_signal_emit (plugin, signals[ADD_SUGGEST], 0, name);
	return TRUE;
}

/**
 * ohm_plugin_set_key:
 *
 **/
gboolean
ohm_plugin_prevent (OhmPlugin   *plugin,
		    const gchar *name)
{
	ohm_debug ("emitting add-prevent '%s'", name);
	g_signal_emit (plugin, signals[ADD_PREVENT], 0, name);
	return TRUE;
}

gboolean
ohm_plugin_preload (OhmPlugin *plugin, const gchar *name)
{
	gchar *path;
	GModule *handle;
	gchar *filename;
	gboolean ret;

	OhmPluginInfo * (*ohm_init_plugin) (OhmPlugin *);

	g_return_val_if_fail (name != NULL, FALSE);

	ohm_debug ("Trying to load : %s", name);

	filename = g_strdup_printf ("libohm_%s.so", name);
	path = getenv ("OHM_PLUGIN_DIR");
	if (path)
		path = g_build_filename (path, filename, NULL);
	else
		path = g_build_filename (LIBDIR, "ohm", filename, NULL);
	g_free (filename);
	handle = g_module_open (path, 0);
	if (!handle) {
		ohm_debug ("opening module %s failed : %s", name, g_module_error ());
		g_free (path);
		return FALSE;
	}
	g_free (path);

	if (!g_module_symbol (handle, "ohm_init_plugin", (gpointer) &ohm_init_plugin)) {
		g_module_close (handle);
		g_error ("could not find init function in plugin");
	}

	plugin->priv->handle = handle;
	plugin->priv->name = g_strdup (name);
	plugin->priv->info = ohm_init_plugin (plugin);

	/* do the load */
	ret = TRUE;
	if (plugin->priv->info->preload != NULL) {
		ret = plugin->priv->info->preload (plugin);
		/* the plugin preload might fail if we do not have the hardware */
	}

	return ret;
}

const gchar *
ohm_plugin_get_name (OhmPlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->priv->name;
}

const gchar *
ohm_plugin_get_version (OhmPlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->priv->info->version;
}

const gchar *
ohm_plugin_get_author (OhmPlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->priv->info->author;
}

G_MODULE_EXPORT gboolean
ohm_plugin_conf_provide (OhmPlugin *plugin,
			 const gchar *name)
{
	GError *error;
	gboolean ret;
	error = NULL;

	ohm_debug ("%s provides %s", plugin->priv->name, name);

	/* provides keys are never public and are always preset at zero */
	ret = ohm_conf_add_key (plugin->priv->conf, name, 0, FALSE, &error);
	if (ret == FALSE) {
		ohm_debug ("Cannot provide key: %s", error->message);
		g_error_free (error);
	}
	return ret;
}

G_MODULE_EXPORT gboolean
ohm_plugin_conf_get_key (OhmPlugin   *plugin,
			 const gchar *key,
			 int         *value)
{
	GError *error;
	gboolean ret;
	error = NULL;
	ret = ohm_conf_get_key (plugin->priv->conf, key, value, &error);
	if (ret == FALSE) {
		ohm_debug ("Cannot get key: %s", error->message);
		g_error_free (error);
	}
	return ret;
}

G_MODULE_EXPORT gboolean
ohm_plugin_conf_set_key (OhmPlugin   *plugin,
			 const gchar *key,
			 int          value)
{
	GError *error;
	gboolean ret;
	error = NULL;

	ret = ohm_conf_set_key_internal (plugin->priv->conf, key, value, TRUE, &error);
	if (ret == FALSE) {
		g_error ("Cannot set key: %s", error->message);
		g_error_free (error);
	}
	return ret;
}

G_MODULE_EXPORT gboolean
ohm_plugin_conf_notify (OhmPlugin   *plugin,
			int          id,
			int          value)
{
	plugin->priv->info->conf_notify (plugin, id, value);
	return TRUE;
}

G_MODULE_EXPORT gboolean
ohm_plugin_coldplug (OhmPlugin   *plugin)
{
	plugin->priv->info->coldplug (plugin);
	return TRUE;
}

/* only use this when required */
G_MODULE_EXPORT gboolean
ohm_plugin_hal_init (OhmPlugin   *plugin)
{
	DBusConnection *conn;

	if (plugin->priv->hal_ctx != NULL) {
		g_warning ("already initialised HAL from this plugin");
		return FALSE;
	}

	/* open a new ctx */
	plugin->priv->hal_ctx = libhal_ctx_new ();
	plugin->priv->hal_property_changed_cb = NULL;
	plugin->priv->hal_condition_cb = NULL;

	/* set the bus connection */
	conn = dbus_bus_get (DBUS_BUS_SYSTEM, NULL);
	libhal_ctx_set_dbus_connection (plugin->priv->hal_ctx, conn);
	libhal_ctx_set_user_data (plugin->priv->hal_ctx, plugin);

	/* connect */
	libhal_ctx_init (plugin->priv->hal_ctx, NULL);

	return TRUE;
}

/* returns -1 for not found */
static gint
ohm_plugin_find_id_from_udi (OhmPlugin *plugin, const gchar *udi)
{
	guint i;
	guint len;
	const gchar *temp_udi;

	len = plugin->priv->hal_udis->len;
	for (i=0; i<len; i++) {
		temp_udi = g_ptr_array_index (plugin->priv->hal_udis, i);
		if (strcmp (temp_udi, udi) == 0) {
			return i;
		}
	}
	return -1;
}

/* returns NULL for not found */
const gchar *
ohm_plugin_find_udi_from_id (OhmPlugin *plugin, guint id)
{
	/* overrange check */
	if (id > plugin->priv->hal_udis->len) {
		return NULL;
	}
	return g_ptr_array_index (plugin->priv->hal_udis, id);
}

/* do something sane and run a function */
static void
hal_property_changed_cb (LibHalContext *ctx,
			 const char *udi,
			 const char *key,
			 dbus_bool_t is_removed,
			 dbus_bool_t is_added)
{
	OhmPlugin *plugin;
	gint id;

	plugin = (OhmPlugin*) libhal_ctx_get_user_data (ctx);
	
	/* find udi in the list so we can emit a easy to parse integer */
	id = ohm_plugin_find_id_from_udi (plugin, udi);
	if (id < 0) {
		/* not watched for this plugin */
		return;
	}

	plugin->priv->hal_property_changed_cb (plugin, id, key);
}

static void
hal_condition_cb (LibHalContext *ctx,
		  const char *udi,
		  const char *name,
		  const char *detail)
{
	OhmPlugin *plugin;
	gint id;
	plugin = (OhmPlugin*) libhal_ctx_get_user_data (ctx);

	/* find udi in the list so we can emit a easy to parse integer */
	id = ohm_plugin_find_id_from_udi (plugin, udi);
	if (id < 0) {
		g_warning ("udi %s not found", udi);
		return;
	}
	plugin->priv->hal_condition_cb (plugin, id, name, detail);
}

G_MODULE_EXPORT gboolean
ohm_plugin_hal_use_property_modified (OhmPlugin	         *plugin,
				      OhmPluginHalPropMod func)
{
	libhal_ctx_set_device_property_modified (plugin->priv->hal_ctx, hal_property_changed_cb);
	plugin->priv->hal_property_changed_cb = func;
	return TRUE;
}

G_MODULE_EXPORT gboolean
ohm_plugin_hal_use_condition (OhmPlugin	           *plugin,
			      OhmPluginHalCondition func)
{
	libhal_ctx_set_device_condition (plugin->priv->hal_ctx, hal_condition_cb);
	plugin->priv->hal_condition_cb = func;
	return TRUE;
}

G_MODULE_EXPORT guint
ohm_plugin_hal_add_device_capability (OhmPlugin   *plugin,
				      const gchar *capability)
{
	gchar **devices;
	gint num_devices;
	guint i;

	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialised from this plugin!");
		return FALSE;
	}

	devices = libhal_find_device_by_capability (plugin->priv->hal_ctx,
						    capability,
						    &num_devices, NULL);

	/* we only support one querying one device with this plugin helper */
	for (i=0; i<num_devices; i++) {
		/* save the udi's in the hash table */
		g_ptr_array_add (plugin->priv->hal_udis, g_strdup (devices[i]));
		/* watch them */
		if (plugin->priv->hal_property_changed_cb != NULL ||
		    plugin->priv->hal_condition_cb != NULL) {
			libhal_device_add_property_watch (plugin->priv->hal_ctx, devices[i], NULL);
		}
	}

	libhal_free_string_array (devices);
	return num_devices;
}

G_MODULE_EXPORT gboolean
ohm_plugin_hal_get_bool (OhmPlugin   *plugin,
			 guint        id,
			 const gchar *key,
			 gboolean    *state)
{
	const gchar *udi;
	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialised from this plugin!");
		return FALSE;
	}
	
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	*state = libhal_device_get_property_bool (plugin->priv->hal_ctx, udi, key, NULL);
	return TRUE;
}

G_MODULE_EXPORT gboolean
ohm_plugin_hal_get_int (OhmPlugin   *plugin,
			guint        id,
			const gchar *key,
			gint        *state)
{
	const gchar *udi;
	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialised from this plugin!");
		return FALSE;
	}
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	*state = libhal_device_get_property_int  (plugin->priv->hal_ctx, udi, key, NULL);
	return TRUE;
}

/* have to free */
G_MODULE_EXPORT gchar *
ohm_plugin_hal_get_udi (OhmPlugin *plugin, guint id)
{
	const gchar *udi;
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	if (udi == NULL) {
		return NULL;
	}
	return g_strdup (udi);
}

G_MODULE_EXPORT gboolean
ohm_plugin_spawn_async (OhmPlugin   *plugin,
			const gchar *commandline)
{
	gboolean ret;
	GError *error;

	error = NULL;
	ohm_debug ("spawning %s", commandline);
	ret = g_spawn_command_line_async (commandline, &error);
	if (ret == FALSE) {
		ohm_debug ("spawn failed: %s", commandline, error->message);
		g_error_free (error);
	}
	return ret;
}

G_MODULE_EXPORT gboolean
ohm_plugin_conf_interested (OhmPlugin   *plugin,
			    const gchar	*key,
			    gint         id)
{
	ohm_debug ("%s provides wants notification of %s on signal %i", plugin->priv->name, key, id);
	g_signal_emit (plugin, signals[ADD_INTERESTED], 0, key, id);
	return TRUE;
}

static void
ohm_plugin_free_hal_table (OhmPlugin *plugin)
{
	guint i;
	guint len;
	gchar *temp_udi;

	len = plugin->priv->hal_udis->len;
	for (i=0; i<len; i++) {
		temp_udi = g_ptr_array_index (plugin->priv->hal_udis, i);
		if (plugin->priv->hal_property_changed_cb != NULL ||
		    plugin->priv->hal_condition_cb != NULL) {
			libhal_device_remove_property_watch (plugin->priv->hal_ctx, temp_udi, NULL);
		}
		g_free (temp_udi);
		g_ptr_array_remove (plugin->priv->hal_udis, temp_udi);
	}
}

/**
 * ohm_plugin_finalize:
 **/
static void
ohm_plugin_finalize (GObject *object)
{
	OhmPlugin *plugin;
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_PLUGIN (object));
	plugin = OHM_PLUGIN (object);

	g_object_unref (plugin->priv->conf);

	if (plugin->priv->info != NULL) {
		if (plugin->priv->info->unload != NULL) {
			plugin->priv->info->unload (plugin);
			/* free hal stuff, if used */
			if (plugin->priv->hal_ctx != NULL) {
				ohm_plugin_free_hal_table (plugin);
				libhal_ctx_shutdown (plugin->priv->hal_ctx, NULL);
			}
		}
	}
	if (plugin->priv->handle != NULL) {
		g_module_close (plugin->priv->handle);
	}

	if (plugin->priv->name != NULL) {
		g_free (plugin->priv->name);
	}
	g_ptr_array_free (plugin->priv->hal_udis, TRUE);
	g_return_if_fail (plugin->priv != NULL);
	G_OBJECT_CLASS (ohm_plugin_parent_class)->finalize (object);
}

/**
 * ohm_plugin_class_init:
 **/
static void
ohm_plugin_class_init (OhmPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_plugin_finalize;

	signals[ADD_INTERESTED] =
		g_signal_new ("add-interested",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmPluginClass, add_interested),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING_INT,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);

	signals[ADD_REQUIRE] =
		g_signal_new ("add-require",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmPluginClass, add_require),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING,
			      G_TYPE_NONE, 1, G_TYPE_STRING);

	signals[ADD_SUGGEST] =
		g_signal_new ("add-suggest",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmPluginClass, add_suggest),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING,
			      G_TYPE_NONE, 1, G_TYPE_STRING);

	signals[ADD_PREVENT] =
		g_signal_new ("add-prevent",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmPluginClass, add_prevent),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING,
			      G_TYPE_NONE, 1, G_TYPE_STRING);

	g_type_class_add_private (klass, sizeof (OhmPluginPrivate));
}

/**
 * ohm_plugin_init:
 **/
static void
ohm_plugin_init (OhmPlugin *plugin)
{
	plugin->priv = OHM_PLUGIN_GET_PRIVATE (plugin);

	plugin->priv->conf = ohm_conf_new ();
	plugin->priv->hal_udis = g_ptr_array_new ();
}

/**
 * ohm_plugin_new:
 **/
OhmPlugin *
ohm_plugin_new (void)
{
	OhmPlugin *plugin;
	plugin = g_object_new (OHM_TYPE_PLUGIN, NULL);
	return OHM_PLUGIN (plugin);
}
