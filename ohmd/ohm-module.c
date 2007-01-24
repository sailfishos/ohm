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
#include "ohm-plugin.h"
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
	gboolean		 doing_preload;
};

/* used as a hash entry type to provide int-passing services to the plugin */
typedef struct {
	OhmPlugin		*plugin;
	gint			 id;
} OhmModuleNofif;

G_DEFINE_TYPE (OhmModule, ohm_module, G_TYPE_OBJECT)

/**
 * ohm_module_process_line:
 **/
static gboolean
ohm_module_process_line (OhmModule   *module,
		         const gchar *line,
		         GSList     **store,
		         const gchar *file_name)
{
	gint len;

	/* check we have a long enough string */
	len = strlen (line);
	if (len < 2) {
		return TRUE;
	}

	/* check to see if we are a comment */
	if (line[0] == '#') {
		return TRUE;
	}

	ohm_debug ("processing from %s store : %s", file_name, line);
	*store = g_slist_prepend (*store, (gpointer) strdup (line));
	
	return TRUE;
}

/**
 * ohm_module_add_initial:
 **/
gboolean
ohm_module_add_initial (OhmModule   *module,
			const gchar *file_name,
			GSList     **store)
{
	gboolean ret;
	gchar *contents;
	gsize length;
	gchar **lines;
	guint i;
	gchar *filename;
	GError *error;

	g_return_val_if_fail (OHM_IS_MODULE (module), FALSE);
	g_return_val_if_fail (file_name != NULL, FALSE);

	/* generate path for each module */
	filename = g_build_path (G_DIR_SEPARATOR_S, SYSCONFDIR, "ohm", file_name, NULL);

	/* load from file */
	error = NULL;
	ret = g_file_get_contents (filename, &contents, &length, &error);
	g_free (filename);
	if (ret == FALSE) {
		/* fixme: display and clear error */
		g_error ("Could not get contents of %s", file_name);
		return FALSE;
	}

	/* split into lines and process each one */
	lines = g_strsplit (contents, "\n", -1);
	i = 0;
	while (lines[i] != NULL) {
		ret = ohm_module_process_line (module, lines[i], store, file_name);
		if (ret == FALSE) {
			g_error ("Loading module info from %s failed", file_name);
		}
		i++;
	}
	g_strfreev (lines);
	g_free (contents);
	return TRUE;
}


static void
key_changed_cb (OhmConf     *conf,
		const gchar *key,
		gint	     value,
		OhmModule   *module)
{
	GSList **entry;
	GSList *l;
	OhmModuleNofif *notif;
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
	for (l=*entry; l != NULL; l=l->next) {
		notif = (OhmModuleNofif *) l->data;
		name = ohm_plugin_get_name (notif->plugin);
		ohm_debug ("notify %s with id:%i", name, notif->id);
		ohm_plugin_conf_notify (notif->plugin, notif->id, value);
	}
}

static void
add_interested_cb (OhmPlugin   *plugin,
		   const gchar *key,
		   gint	        id,
		   OhmModule   *module)
{
	GSList **entry;
	GSList **l;
	OhmModuleNofif *notif;
	ohm_debug ("add interested! %s : %i", key, id);

	/* if present, add to SList, if not, add to hash as slist object */
	entry = g_hash_table_lookup (module->priv->interested, key);

	/* create a new notifier, and copy over the data */
	notif = g_new0 (OhmModuleNofif, 1); /* TODO: use gslice */
	notif->plugin = plugin;
	notif->id = id;

	if (entry != NULL) {
		/* already present, just append to SList */
		ohm_debug ("key already watched by someting else");
		*entry = g_slist_prepend (*entry, (gpointer) notif);
	} else {
		ohm_debug ("key not already watched by someting else");
		/* create the new SList andd add the new notification to it */
		l = g_new0 (GSList *, 1);
		*l = NULL;
		*l = g_slist_prepend (*l, (gpointer) notif);
		/* fixme we need to free this g_strdup at finalize and clear the list */
		g_hash_table_insert (module->priv->interested, (gpointer) g_strdup (key), l);
	}
}

static void
add_require_cb (OhmPlugin   *plugin,
		const gchar *name,
		OhmModule   *module)
{
	if (module->priv->doing_preload == FALSE) {
		g_error ("modules not allowed to call ohm_plugin_require() after load()");
	}
	ohm_debug ("adding module require %s", name);
	module->priv->mod_require = g_slist_prepend (module->priv->mod_require, (gpointer) strdup (name));
}

static void
add_suggest_cb (OhmPlugin   *plugin,
		const gchar *name,
		OhmModule   *module)
{
	if (module->priv->doing_preload == FALSE) {
		g_error ("modules not allowed to call ohm_suggest_require() after load()");
	}
	ohm_debug ("adding module suggest %s", name);
	module->priv->mod_suggest = g_slist_prepend (module->priv->mod_suggest, (gpointer) strdup (name));
}

static void
add_prevent_cb (OhmPlugin   *plugin,
		const gchar *name,
		OhmModule   *module)
{
	if (module->priv->doing_preload == FALSE) {
		g_error ("modules not allowed to call ohm_plugin_prevent() after load()");
	}
	ohm_debug ("adding module prevent %s", name);
	module->priv->mod_prevent = g_slist_prepend (module->priv->mod_prevent, (gpointer) strdup (name));
}

static gboolean
ohm_module_add_plugin (OhmModule *module, const gchar *name)
{
	OhmPlugin *plugin;
	gboolean ret;

	/* setup new plugin */
	plugin = ohm_plugin_new ();
	g_signal_connect (plugin, "add-interested",
			  G_CALLBACK (add_interested_cb), module);
	g_signal_connect (plugin, "add-require",
			  G_CALLBACK (add_require_cb), module);
	g_signal_connect (plugin, "add-suggest",
			  G_CALLBACK (add_suggest_cb), module);
	g_signal_connect (plugin, "add-prevent",
			  G_CALLBACK (add_prevent_cb), module);
	/* try to load plugin, this might fail */
	ret = ohm_plugin_load (plugin, name);
	if (ret == TRUE) {
		ohm_debug ("adding %s to module list", name);
		module->priv->plugins = g_slist_prepend (module->priv->plugins, (gpointer) plugin);
	} else {
		/* if it does, just unref and warn */
		ohm_debug ("not adding %s to module list as cannot load", name);
		g_object_unref (plugin);
	}
	return ret;
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
 * ohm_module_finalize:
 **/
static void
ohm_module_finalize (GObject *object)
{
	OhmModule *module;
	GSList *l;
	OhmPlugin *plugin;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MODULE (object));
	module = OHM_MODULE (object);

	g_hash_table_destroy (module->priv->interested);
	g_object_unref (module->priv->conf);

	/* unref each plugin */
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		g_object_unref (plugin);
	}

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
	/* clear lists */
	module->priv->mod_require = NULL;
	module->priv->mod_suggest = NULL;
	module->priv->mod_prevent = NULL;
	module->priv->mod_loaded = NULL;

	module->priv->interested = g_hash_table_new (g_str_hash, g_str_equal);

	module->priv->conf = ohm_conf_new ();
	g_signal_connect (module->priv->conf, "key-changed",
			  G_CALLBACK (key_changed_cb), module);

	/* do the plugin load, with dependencies */
	module->priv->doing_preload = TRUE;
	ohm_module_add_initial (module, "require", &(module->priv->mod_require));
	ohm_module_add_initial (module, "suggest", &(module->priv->mod_suggest));
	ohm_module_add_initial (module, "prevent", &(module->priv->mod_prevent));

	/* Keep trying to empty both require and suggested lists.
	 * We could have done this recursively, but that is really bad for the stack.
	 * We also have to keep in mind the lists may be being updated by plugins as we load them */
	i = 1;
	while (module->priv->mod_require != NULL ||
	       module->priv->mod_suggest != NULL) {
		ohm_debug ("coldplug iteration #%i", i++);
		ohm_module_add_all_plugins (module);
		if (i > 10) {
			g_error ("coldplug too complex, please file a bug");
		}
	}
	module->priv->doing_preload = FALSE;

	/* add defaults for each plugin before the coldplug */
	ohm_debug ("starting plugin coldplug");
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

		ohm_plugin_coldplug (plugin);
	}

	/* coldplug each plugin */
	ohm_debug ("starting plugin coldplug");
	for (l=module->priv->plugins; l != NULL; l=l->next) {
		plugin = (OhmPlugin *) l->data;
		name = ohm_plugin_get_name (plugin);
		ohm_debug ("coldplug %s", name);
		ohm_plugin_coldplug (plugin);
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
