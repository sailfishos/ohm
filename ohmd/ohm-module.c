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

#include "ohm-module.h"
#include "ohm-plugin.h"
#include "ohm-conf.h"

#define OHM_MODULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MODULE, OhmModulePrivate))

struct OhmModulePrivate
{
	GSList			*module_require;
	GSList			*module_suggest;
	GSList			*module_prevent;
	GSList			*module_loaded;
	GHashTable		*interested;
	OhmPlugin		*plugin; /* needs to be a list */
	OhmConf			*conf;
};

/* used as a hash entry type to provide int-passing services to the plugin */
typedef struct {
	OhmPlugin		*plugin;
	gint			 id;
} OhmModuleNofif;

G_DEFINE_TYPE (OhmModule, ohm_module, G_TYPE_OBJECT)

/**
 * ohm_module_require:
 **/
gboolean
ohm_module_require (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("module:require '%s'", name);
	return TRUE;
}

/**
 * ohm_module_suggest:
 **/
gboolean
ohm_module_suggest (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("module:suggest '%s'", name);
	return TRUE;
}

/**
 * ohm_module_prevent:
 *
 **/
gboolean
ohm_module_prevent (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("module:prevent '%s'", name);
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

	g_debug ("module:key changed! %s : %i", key, value);

	/* if present, add to SList, if not, add to hash as slist object */
	entry = g_hash_table_lookup (module->priv->interested, key);

	/* a key has changed that none of the plugins are watching */
	if (entry == NULL) {
		return;
	}

	g_debug ("module:found watched key %s", key);
	/* go thru the SList and notify each plugin */
	for (l=*entry; l != NULL; l=l->next) {
		notif = (OhmModuleNofif *) l->data;
		name = ohm_plugin_get_name (notif->plugin);
		g_debug ("module:notify %s with id:%i", name, notif->id);
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
	g_debug ("module:add interested! %s : %i", key, id);

	/* if present, add to SList, if not, add to hash as slist object */
	entry = g_hash_table_lookup (module->priv->interested, key);

	/* create a new notifier, and copy over the data */
	notif = g_new0 (OhmModuleNofif, 1); /* TODO: use gslice */
	notif->plugin = plugin;
	notif->id = id;

	if (entry != NULL) {
		/* already present, just append to SList */
		g_debug ("module:key already watched by someting else");
		*entry = g_slist_prepend (*entry, (gpointer) notif);
	} else {
		g_debug ("module:key not already watched by someting else");
		/* create the new SList andd add the new notification to it */
		l = g_new0 (GSList *, 1);
		*l = NULL;
		*l = g_slist_prepend (*l, (gpointer) notif);
		/* fixme we need to free this g_strdup at finalize and clear the list */
		g_hash_table_insert (module->priv->interested, (gpointer) g_strdup (key), l);
	}
}

/**
 * ohm_module_finalize:
 **/
static void
ohm_module_finalize (GObject *object)
{
	OhmModule *module;
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MODULE (object));
	module = OHM_MODULE (object);

	g_hash_table_destroy (module->priv->interested);
	g_object_unref (module->priv->conf);
	g_object_unref (module->priv->plugin);

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
	module->priv = OHM_MODULE_GET_PRIVATE (module);
	/* clear lists */
	module->priv->module_require = NULL;
	module->priv->module_suggest = NULL;
	module->priv->module_prevent = NULL;
	module->priv->module_loaded = NULL;

	module->priv->interested = g_hash_table_new (g_str_hash, g_str_equal);

	module->priv->conf = ohm_conf_new ();
	g_signal_connect (module->priv->conf, "key-changed",
			  G_CALLBACK (key_changed_cb), module);

	/* play for now, we really need to read in from disk a list of modules to load */
	module->priv->plugin = ohm_plugin_new ();
	g_signal_connect (module->priv->plugin, "add-interested",
			  G_CALLBACK (add_interested_cb), module);

	ohm_plugin_load (module->priv->plugin, "libpluginbattery.so");
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
