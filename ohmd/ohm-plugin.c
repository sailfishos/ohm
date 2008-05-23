/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2007 Codethink Ltd
 *            Author: Rob Taylor <rob.taylor@codethink.co.uk>
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
#include "ohm-plugin-internal.h"
#include "ohm-conf.h"
#include "ohm-marshal.h"
#include "ohm-dbus.h"

#define OHM_PLUGIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_PLUGIN, OhmPluginPrivate))

struct _OhmPluginPrivate
{
	OhmConf			*conf;
	GModule			*handle;
	gchar			*name;
	/* not assigned unless a plugin uses hal */
	LibHalContext		*hal_ctx;
	GPtrArray		*hal_udis;
	OhmPluginHalPropMod	 hal_property_changed_cb;
	OhmPluginHalCondition	 hal_condition_cb;
	const char		*key_being_set;
};

G_DEFINE_TYPE (OhmPlugin, ohm_plugin, G_TYPE_OBJECT)


gboolean
ohm_plugin_load (OhmPlugin *plugin, const gchar *name)
{
	gchar *path;
	GModule *handle;
	gchar *filename;
	char buf[128];

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

	
#define SYMBOL(descr, sym, field, required) do {			\
	  if (!g_module_symbol((handle), sym, (gpointer)&plugin->field) && \
	      required)	{						\
	    ohm_debug("could not find %s in plugin %s, not loading",	\
		      (descr), sym);					\
	    g_module_close(handle);					\
	    return FALSE;						\
	  }								\
	} while (0)

	SYMBOL("description", "ohm_plugin_desc"      , desc      , 1);
	SYMBOL("interested" , "ohm_plugin_interested", interested, 0);
	SYMBOL("provides"   , "ohm_plugin_provides"  , provides  , 0);
	SYMBOL("requires"   , "ohm_plugin_requires"  , requires  , 0);
	SYMBOL("suggests"   , "ohm_plugin_suggests"  , suggests  , 0);
	SYMBOL("prevents"   , "ohm_plugin_prevents"  , prevents  , 0);
	
	snprintf(buf, sizeof(buf), "%s%s", name, OHM_EXPORT_VAR);
	SYMBOL("exports", buf, desc->exports, 0);
	snprintf(buf, sizeof(buf), "%s%s", name, OHM_IMPORT_VAR);
	SYMBOL("imports", buf, desc->imports, 0);

	SYMBOL("dbus methods", "ohm_plugin_dbus_methods", dbus_methods, 0);
	SYMBOL("dbus signals", "ohm_plugin_dbus_signals", dbus_signals, 0);
	
	plugin->priv->handle = handle;
	plugin->priv->name = g_strdup (name);
	return TRUE;
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

	return plugin->desc->version;
}

const gchar *
ohm_plugin_get_author (OhmPlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, NULL);

	return plugin->desc->author;
}

gboolean
ohm_plugin_conf_get_key (OhmPlugin   *plugin,
			const gchar  *key,
			int	     *value)
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

gboolean
ohm_plugin_conf_set_key (OhmPlugin   *plugin,
			const gchar  *key,
			int	      value)
{
	GError *error;
	gboolean ret;
	error = NULL;

	/* key_being_set is used to stop a plugin changing a key notifying
	 * itself if it's interest in that key
	 */
	plugin->priv->key_being_set = key;
	ohm_debug ("plugin %s setting key %s to %d", plugin->priv->name, key, value);
	ret = ohm_conf_set_key_internal (plugin->priv->conf, key, value, TRUE, &error);
	plugin->priv->key_being_set = NULL;

	if (ret == FALSE) {
		g_error ("Cannot set key: %s", error->message);
		g_error_free (error);
	}
	return ret;
}

gboolean
ohm_plugin_notify (OhmPlugin   *plugin,
		   const char *key,
		   int     id,
		   int     value)
{
	/* check that it wasn't this plugin that changed the key in 
	 * the first place
	 */
	if (plugin->priv->key_being_set &&
	    strcmp(plugin->priv->key_being_set, key) == 0)
		return TRUE;

	plugin->desc->notify (plugin, id, value);
	return TRUE;
}

gboolean
ohm_plugin_initialize (OhmPlugin   *plugin)
{
	if (plugin->desc->initialize)
		plugin->desc->initialize (plugin);
	return TRUE;
}

ohm_method_t *
ohm_plugin_exports (OhmPlugin *plugin)
{
  if (plugin->desc->exports)
    return plugin->desc->exports;
  else
    return NULL;
}

ohm_method_t *
ohm_plugin_imports (OhmPlugin *plugin)
{
  if (plugin->desc->imports)
    return plugin->desc->imports;
  else
    return NULL;
}

/* only use this when required */
gboolean
ohm_plugin_hal_init (OhmPlugin   *plugin)
{
	DBusConnection *conn;

	if (plugin->priv->hal_ctx != NULL) {
		g_warning ("already initialized HAL from this plugin");
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

gboolean
ohm_plugin_hal_use_property_modified (OhmPlugin	         *plugin,
				      OhmPluginHalPropMod func)
{
	libhal_ctx_set_device_property_modified (plugin->priv->hal_ctx, hal_property_changed_cb);
	plugin->priv->hal_property_changed_cb = func;
	return TRUE;
}

gboolean
ohm_plugin_hal_use_condition (OhmPlugin	           *plugin,
			      OhmPluginHalCondition func)
{
	libhal_ctx_set_device_condition (plugin->priv->hal_ctx, hal_condition_cb);
	plugin->priv->hal_condition_cb = func;
	return TRUE;
}

guint
ohm_plugin_hal_add_device_capability (OhmPlugin   *plugin,
				      const gchar *capability)
{
	gchar **devices;
	gint num_devices;
	guint i;

	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialized from this plugin!");
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

gboolean
ohm_plugin_hal_get_bool (OhmPlugin   *plugin,
			 guint        id,
			 const gchar *key,
			 gboolean    *state)
{
	const gchar *udi;
	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialized from this plugin!");
		return FALSE;
	}
	
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	*state = libhal_device_get_property_bool (plugin->priv->hal_ctx, udi, key, NULL);
	return TRUE;
}

gboolean
ohm_plugin_hal_get_int (OhmPlugin   *plugin,
			guint        id,
			const gchar *key,
			gint        *state)
{
	const gchar *udi;
	if (plugin->priv->hal_ctx == NULL) {
		g_warning ("HAL not already initialized from this plugin!");
		return FALSE;
	}
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	*state = libhal_device_get_property_int  (plugin->priv->hal_ctx, udi, key, NULL);
	return TRUE;
}

/* have to free */
gchar *
ohm_plugin_hal_get_udi (OhmPlugin *plugin, guint id)
{
	const gchar *udi;
	udi = ohm_plugin_find_udi_from_id (plugin, id);
	if (udi == NULL) {
		return NULL;
	}
	return g_strdup (udi);
}

gboolean
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
	}
}


/**
 * ohm_plugin_dbus_get_connection:
 **/
DBusConnection *
ohm_plugin_dbus_get_connection(void)
{
  return ohm_dbus_get_connection();
}


/**
 * ohm_plugin_dbus_add_method:
 **/
int
ohm_plugin_dbus_add_method(const char *path, const char *name,
			   DBusObjectPathMessageFunction handler, void *data)
{
  return ohm_dbus_add_method(path, name, handler, data);
}


/**
 * ohm_plugin_dbus_del_method:
 **/
int
ohm_plugin_dbus_del_method(const char *path, const char *member,
			   DBusObjectPathMessageFunction handler, void *data)
{
  return ohm_dbus_del_method(path, member, handler, data);
}


/**
 * ohm_plugin_dbus_add_signal:
 **/
int
ohm_plugin_dbus_add_signal(const char *sender, const char *interface,
			   const char *sig, const char *path,
			   DBusObjectPathMessageFunction handler, void *data)
{
  return ohm_dbus_add_signal(sender, interface, sig, path, handler, data);
}


/**
 * ohm_plugin_dbus_del_signal:
 **/
void
ohm_plugin_dbus_del_signal(const char *sender, const char *interface,
			   const char *sig, const char *path,
			   DBusObjectPathMessageFunction handler, void *data)
{
  return ohm_dbus_del_signal(sender, interface, sig, path, handler, data);
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

	if (plugin->desc != NULL) {
		if (plugin->desc->destroy != NULL) {
			plugin->desc->destroy (plugin);
		}
		/* free hal stuff, if used */
		if (plugin->priv->hal_ctx != NULL) {
			ohm_plugin_free_hal_table (plugin);
			libhal_ctx_shutdown (plugin->priv->hal_ctx, NULL);
			libhal_ctx_free (plugin->priv->hal_ctx);
		}
		
	}

	if (plugin->priv->name != NULL) {
		g_free (plugin->priv->name);
	}
	g_ptr_array_free (plugin->priv->hal_udis, TRUE);

	g_debug ("g_module_close(%p)", plugin->priv->handle);
	g_module_close (plugin->priv->handle);

	G_OBJECT_CLASS (ohm_plugin_parent_class)->finalize (object);
}

/**
 * ohm_plugin_class_init:
 **/
static void
ohm_plugin_class_init (OhmPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ohm_plugin_finalize;
	g_type_class_add_private (klass, sizeof (OhmPluginPrivate));
}

/**
 * ohm_plugin_init:
 **/
static void
ohm_plugin_init (OhmPlugin *plugin)
{
	plugin->priv = OHM_PLUGIN_GET_PRIVATE (plugin);

	plugin->priv->hal_udis = g_ptr_array_new ();
	plugin->priv->conf = ohm_conf_new ();
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
