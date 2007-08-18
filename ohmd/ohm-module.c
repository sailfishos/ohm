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

#include "ohm-debug.h"
#include "ohm-module.h"
#include "ohm-plugin-internal.h"
#include "ohm-conf.h"

#define OHM_MODULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MODULE, OhmModulePrivate))

struct OhmModulePrivate
{
	GSList			*mod_require;
	GSList			*mod_suggest;
	GSList			*mod_prevent;
	GSList			*mod_loaded;	/* list of loaded module names */
	GSList			*plugins;	/* list of loaded OhmPlugin's */
	GHashTable		*interested;
	OhmConf			*conf;
	gboolean		 do_extra_checks;
	gchar			**modules_banned;
	gchar			**modules_suggested;
	gchar			**modules_required;
};

/* used as a hash entry type to provide int-passing services to the plugin */
typedef struct {
	OhmPlugin		*plugin;
	gint			 id;
} OhmModuleNotify;

G_DEFINE_TYPE (OhmModule, ohm_module, G_TYPE_OBJECT)

static gboolean
free_notify_list (const gchar *key, GSList *list, gpointer userdata)
{
	GSList *l;

	for (l=list; l != NULL; l=l->next) {
		g_slice_free (OhmModuleNotify, l->data);
	}
	g_slist_free (list);

	return TRUE;
}

static void
key_changed_cb (OhmConf     *conf,
		const gchar *key,
		gint	     value,
		OhmModule   *module)
{
	GSList *entry;
	GSList *l;
	OhmModuleNotify *notif;
	const gchar *name;

	ohm_debug ("key changed! %s : %i", key, value);

	/* if present, add to SList, if not, add to hash as slist object */
	entry = g_hash_table_lookup (module->priv->interested, key);

	/* a key has changed that none of the plugins are watching */
	if (entry == NULL) {
		return;
	}

	ohm_debug ("found watched key %s", key);
	/* go thru the SList and notify each plugin */
	for (l=entry; l != NULL; l=l->next) {
		notif = (OhmModuleNotify *) l->data;
		name = ohm_plugin_get_name (notif->plugin);
		ohm_debug ("notify %s with id:%i", name, notif->id);
		ohm_plugin_notify (notif->plugin, key, notif->id, value);
	}
}

static void
add_interesteds (OhmModule   *module, OhmPlugin   *plugin)
{
	GSList *entry;
	OhmModuleNotify *notif;
	const OhmPluginKeyIdMap *interested;
	
	if (plugin->interested == NULL)
		return;

	for (interested = plugin->interested; interested->key_name; interested++) {
		ohm_debug ("add interested! %s : %i", interested->key_name, interested->local_key_id);

		/* if present, add to SList, if not, add to hash as slist object */
		entry = g_hash_table_lookup (module->priv->interested, interested->key_name);

		/* create a new notifier, and copy over the data */
		notif = g_slice_new (OhmModuleNotify);
		notif->plugin = plugin;
		notif->id = interested->local_key_id;

		entry = g_slist_prepend (entry, (gpointer) notif);
		g_hash_table_insert (module->priv->interested, (gpointer) interested->key_name, entry);
	}
}


static gboolean
add_provides (OhmModule *module, OhmPlugin *plugin)
{
	GError *error;
	const char **provides = plugin->provides;
	gboolean ret=TRUE;
	error = NULL;

	if (provides == NULL)
		return TRUE;

	for (; *provides; provides++) {
		ohm_debug ("%s provides %s", ohm_plugin_get_name(plugin), *provides);
		/* provides keys are never public and are always preset at zero */
		ret &= ohm_conf_add_key (module->priv->conf, *provides, 0, FALSE, &error);
		if (ret == FALSE) {
			ohm_debug ("Cannot provide key %s: %s", *provides, error->message);
			g_error_free (error);
		}
	}
	return ret;
}

static void
add_names (GSList **l, const char **names)
{
	if (names == NULL)
		return;

	for (;*names; names++) {
		*l = g_slist_prepend (*l, (gpointer) *names);
	}
}

static gboolean
ohm_module_add_plugin (OhmModule *module, const gchar *name)
{
	OhmPlugin *plugin;

	/* setup new plugin */
	plugin = ohm_plugin_new ();

	/* try to load plugin, this might fail */
	if (!ohm_plugin_load (plugin, name))
		return FALSE;

	ohm_debug ("adding %s to module list", name);
	module->priv->plugins = g_slist_prepend (module->priv->plugins, (gpointer) plugin);
	add_names (&module->priv->mod_require, plugin->requires);
	add_names (&module->priv->mod_suggest, plugin->suggests);
	add_names (&module->priv->mod_prevent, plugin->prevents);
	add_interesteds (module, plugin);

	if (!add_provides (module, plugin))
		return FALSE;
	else
		return TRUE;
}

/* adds plugins from require and suggests lists. Failure of require is error, failure of suggests is warning */
/* we have to make sure we do not load banned plugins from the prevent list or load already loaded plugins */
/* this should be very fast (two or three runs) for the common case */
static void
ohm_module_add_all_plugins (OhmModule *module)
{
	GSList *lfound;
	GSList *current;
	gchar *entry;
	gboolean ret;

	/* go through requires */
	if (module->priv->mod_require != NULL) {
		ohm_debug ("processing require");
	}
	while (module->priv->mod_require != NULL) {
		current = module->priv->mod_require;
		entry = (gchar *) current->data;

		/* make sure it's not banned */
		lfound = g_slist_find_custom (module->priv->mod_prevent, entry, (GCompareFunc) strcmp);
		if (lfound != NULL) {
			g_error ("module listed in require is also listed in prevent");
		}

		/* make sure it's not already loaded */
		lfound = g_slist_find_custom (module->priv->mod_loaded, entry, (GCompareFunc) strcmp);
		if (lfound == NULL) {
			/* load module */
			ret = ohm_module_add_plugin (module, entry);
			if (ret == FALSE) {
				g_error ("module %s failed to load but listed in require", entry);
			}

			/* add to loaded list */
			module->priv->mod_loaded = g_slist_prepend (module->priv->mod_loaded, (gpointer) entry);
		} else {
			ohm_debug ("module %s already loaded", entry);
		}

		/* remove this entry from the list, and use cached current as the head may have changed */
		module->priv->mod_require = g_slist_delete_link (module->priv->mod_require, current);
	}

	/* go through suggest */
	if (module->priv->mod_suggest != NULL) {
		ohm_debug ("processing suggest");
	}
	while (module->priv->mod_suggest != NULL) {
		current = module->priv->mod_suggest;
		entry = (gchar *) current->data;

		/* make sure it's not banned */
		lfound = g_slist_find_custom (module->priv->mod_prevent, entry, (GCompareFunc) strcmp);
		if (lfound != NULL) {
			ohm_debug ("module %s listed in suggest is also listed in prevent, so ignoring", entry);
		} else {
			/* make sure it's not already loaded */
			lfound = g_slist_find_custom (module->priv->mod_loaded, entry, (GCompareFunc) strcmp);
			if (lfound == NULL) {
				ohm_debug ("try add: %s", entry);
				/* load module */
				ret = ohm_module_add_plugin (module, entry);
				if (ret == TRUE) {
					/* add to loaded list */
					module->priv->mod_loaded = g_slist_prepend (module->priv->mod_loaded, (gpointer) entry);
				} else {
					ohm_debug ("module %s failed to load but only suggested so no problem", entry);
				}
			}
		}
	
		/* remove this entry from the list */
		module->priv->mod_suggest = g_slist_delete_link (module->priv->mod_suggest, current);
	}
}

/**
 * ohm_module_read_defaults:
 **/
static void
ohm_module_read_defaults (OhmModule *module)
{
	GKeyFile *keyfile;
	gchar *filename;
	gchar *conf_dir;
	gsize length;
	guint i;
	GError *error;
	gboolean ret;

	/* use g_key_file. It's quick, and portable */
	keyfile = g_key_file_new ();

	/* generate path for conf file */
	conf_dir = getenv ("OHM_CONF_DIR");
	if (conf_dir != NULL) {
		/* we have from the environment */
		filename = g_build_path (G_DIR_SEPARATOR_S, conf_dir, "modules.ini", NULL);
	} else {
		/* we are running as normal */
		filename = g_build_path (G_DIR_SEPARATOR_S, SYSCONFDIR, "ohm", "modules.ini", NULL);
	}
	ohm_debug ("keyfile = %s", filename);

	/* we can never save the file back unless we remove G_KEY_FILE_NONE */
	error = NULL;
	ret = g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, &error);
	if (ret == FALSE) {
		g_error ("cannot load keyfile %s", filename);
	}
	g_free (filename);

	error = NULL;
	module->priv->do_extra_checks = g_key_file_get_boolean (keyfile, "Modules", "PerformExtraChecks", &error);
	if (error != NULL) {
		ohm_debug ("PerformExtraChecks read error: %s", error->message);
		g_error_free (error);
	}
	ohm_debug ("PerformExtraChecks=%i", module->priv->do_extra_checks);

	/* read and process ModulesBanned */
	error = NULL;
	module->priv->modules_banned = g_key_file_get_string_list (keyfile, "Modules", "ModulesBanned", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesBanned read error: %s", error->message);
		g_error_free (error);
	}
	for (i=0; i<length; i++) {
		ohm_debug ("ModulesBanned: %s", module->priv->modules_banned[i]);
		module->priv->mod_prevent = g_slist_prepend (module->priv->mod_prevent, (gpointer) module->priv->modules_banned[i]);
	}

	/* read and process ModulesSuggested */
	error = NULL;
	module->priv->modules_suggested = g_key_file_get_string_list (keyfile, "Modules", "ModulesSuggested", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesSuggested read error: %s", error->message);
		g_error_free (error);
	}
	for (i=0; i<length; i++) {
		ohm_debug ("ModulesSuggested: %s", module->priv->modules_suggested[i]);
		module->priv->mod_suggest = g_slist_prepend (module->priv->mod_suggest, (gpointer) module->priv->modules_suggested[i]);
	}

	/* read and process ModulesRequired */
	error = NULL;
	module->priv->modules_required = g_key_file_get_string_list (keyfile, "Modules", "ModulesRequired", &length, &error);
	if (error != NULL) {
		ohm_debug ("ModulesRequired read error: %s", error->message);
		g_error_free (error);
	}
	for (i=0; i<length; i++) {
		ohm_debug ("ModulesRequired: %s", module->priv->modules_required[i]);
		module->priv->mod_require = g_slist_prepend (module->priv->mod_require, (gpointer) module->priv->modules_required[i]);
	}

	g_key_file_free (keyfile);
}

static void
ohm_module_finalize (GObject *object)
{
	OhmModule *module;
	GSList *l;
	OhmPlugin *plugin;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MODULE (object));
	module = OHM_MODULE (object);

	g_hash_table_foreach_remove (module->priv->interested, (GHRFunc) free_notify_list, NULL);
	g_hash_table_destroy (module->priv->interested);
	g_object_unref (module->priv->conf);

	/* unref each plugin */
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		g_object_unref (plugin);
	}
	g_slist_free (module->priv->plugins);

	g_return_if_fail (module->priv != NULL);
	G_OBJECT_CLASS (ohm_module_parent_class)->finalize (object);
}

/**
 * ohm_module_class_init:
 **/
static void
ohm_module_class_init (OhmModuleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_module_finalize;

	g_type_class_add_private (klass, sizeof (OhmModulePrivate));
}

/**
 * ohm_module_init:
 **/
static void
ohm_module_init (OhmModule *module)
{
	guint i;
	GSList *l;
	OhmPlugin *plugin;
	const gchar *name;
	GError *error;
	gboolean ret;

	module->priv = OHM_MODULE_GET_PRIVATE (module);

	module->priv->interested = g_hash_table_new (g_str_hash, g_str_equal);

	module->priv->conf = ohm_conf_new ();
	g_signal_connect (module->priv->conf, "key-changed",
			  G_CALLBACK (key_changed_cb), module);

	/* read the defaults in from modules.ini */
	ohm_module_read_defaults (module);

	/* Keep trying to empty both require and suggested lists.
	 * We could have done this recursively, but that is really bad for the stack.
	 * We also have to keep in mind the lists may be being updated by plugins as we load them */
	i = 1;
	while (module->priv->mod_require != NULL ||
	       module->priv->mod_suggest != NULL) {
		ohm_debug ("module add iteration #%i", i++);
		ohm_module_add_all_plugins (module);
		if (i > 10) {
			g_error ("Module add too complex, please file a bug");
		}
	}
	g_slist_free (module->priv->mod_prevent);
	g_slist_free (module->priv->mod_loaded);
	g_strfreev (module->priv->modules_required);
	g_strfreev (module->priv->modules_suggested);
	g_strfreev (module->priv->modules_banned);

	/* add defaults for each plugin before the initialization*/
	ohm_debug ("loading plugin defaults");
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		name = ohm_plugin_get_name (plugin);
		ohm_debug ("load defaults %s", name);

		/* load defaults from disk */
		error = NULL;
		ret = ohm_conf_load_defaults (module->priv->conf, name, &error);
		if (ret == FALSE) {
			g_error ("could not load defaults : %s", error->message);
			g_error_free (error);
		}
	}

	/* initialize each plugin */
	ohm_debug ("starting plugin initialization");
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		name = ohm_plugin_get_name (plugin);
		ohm_debug ("initialize %s", name);
		ohm_plugin_initialize (plugin);
	}
}

/**
 * ohm_module_new:
 **/
OhmModule *
ohm_module_new (void)
{
	OhmModule *module;
	module = g_object_new (OHM_TYPE_MODULE, NULL);
	return OHM_MODULE (module);
}
