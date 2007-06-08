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

#include "ohm-debug.h"
#include "ohm-conf.h"
#include "ohm-confobj.h"
#include "ohm-marshal.h"

#define OHM_CONF_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_CONF, OhmConfPrivate))

struct OhmConfPrivate
{
	GHashTable		*keys;
};

enum {
	KEY_ADDED,
	KEY_CHANGED,
	LAST_SIGNAL
};

static guint	     signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (OhmConf, ohm_conf, G_TYPE_OBJECT)

/**
 * ohm_conf_error_quark:
 * Return value: Our personal error quark.
 **/
GQuark
ohm_conf_error_quark (void)
{
	static GQuark quark = 0;
	if (!quark) {
		quark = g_quark_from_static_string ("ohm_conf_error");
	}
	return quark;
}

/**
 * ohm_conf_compare_func
 * A GCompareFunc for comparing two OhmConfKeyValue objects by name.
 **/
static gint
ohm_conf_compare_func (gconstpointer a, gconstpointer b)
{
	OhmConfKeyValue *entry1 = (OhmConfKeyValue *) a;
	OhmConfKeyValue *entry2 = (OhmConfKeyValue *) b;
	return strcmp (entry1->name, entry2->name);
}

/**
 * ohm_conf_to_slist_iter:
 **/
static void
ohm_conf_to_slist_iter (gpointer    key,
			OhmConfObj *confobj,
			gpointer   *user_data)
{
	OhmConfKeyValue *keyvalue;
	GSList **list;

	/* copy key values into the ABI stable struct to export */
	keyvalue = g_new0 (OhmConfKeyValue, 1);
	keyvalue->name = ohm_confobj_get_key (confobj);
	keyvalue->value = ohm_confobj_get_value (confobj);
	keyvalue->public = ohm_confobj_get_public (confobj);

	/* add to list */
	list = (GSList **) user_data;
	*list = g_slist_prepend (*list, (gpointer) keyvalue);
}

/**
 * ohm_conf_get_all:
 *
 * Gets an ordered list of all the key values in OhmConfKeyValue's.
 * Free the list with ohm_conf_free_keys().
 **/
gboolean
ohm_conf_get_keys (OhmConf *conf,
		   GSList **list)
{
	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (*list == NULL, FALSE);

	ohm_debug ("Get all keys and values in database");
	if (conf->priv->keys == NULL) {
		g_warning ("Conf invalid");
		return FALSE;
	}

	/* add hash to unsorted SList */
	g_hash_table_foreach (conf->priv->keys, (GHFunc) ohm_conf_to_slist_iter, list);

	/* sort list */
	*list = g_slist_sort (*list, ohm_conf_compare_func);
	return TRUE;
}

/**
 * ohm_conf_get_all:
 *
 * Gets an ordered list of all the key values in OhmConfKeyValue's.
 * Make sure to delete the list after it's been used with g_slist_free()
 **/
gboolean
ohm_conf_free_keys (OhmConf *conf,
		    GSList  *list)
{
	GSList *l;
	OhmConfKeyValue *keyvalue;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (list != NULL, FALSE);

	/* free the object only, text is an internal pointer */
	for (l=list; l != NULL; l=l->next) {
		keyvalue = (OhmConfKeyValue *) l->data;
		g_free (keyvalue);
	}
	g_slist_free (list);

	return TRUE;
}

/**
 * ohm_conf_get_key:
 **/
gboolean
ohm_conf_get_key (OhmConf     *conf,
		  const gchar *key,
		  gint        *value,
		  GError     **error)
{
	OhmConfObj *confobj;
	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (value != NULL, FALSE);
	g_return_val_if_fail (error != NULL, FALSE);
	g_return_val_if_fail (*error == NULL, FALSE);

	if (conf->priv->keys == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_INVALID,
				      "Conf invalid");
		*value = 0;
		return FALSE;
	}

	/* try to find the key in the global conf */
	confobj = g_hash_table_lookup (conf->priv->keys, key);
	if (confobj == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_MISSING,
				      "Key %s missing", key);
		*value = 0;
		return FALSE;
	}

	/* copy value from key */
	*value = ohm_confobj_get_value (confobj);
	return TRUE;
}

/**
 * ohm_conf_set_key:
 * internal set true for plugin access and false for public dbus
 *
 **/
gboolean
ohm_conf_add_key (OhmConf     *conf,
		  const gchar *key,
		  gint         value,
		  gboolean     public,
		  GError     **error)
{
	OhmConfObj *confobj;
	gchar *confkey;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (conf->priv->keys == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_INVALID,
				      "Conf invalid");
		return FALSE;
	}

	/* try to find the key in the global conf */
	confobj = g_hash_table_lookup (conf->priv->keys, key);
	if (confobj != NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_ALREADY_EXISTS,
				      "Key %s already exists", key);
		return FALSE;
	}

	/* create a new key */
	ohm_debug ("create key '%s' : %i", key, value);
	confobj = ohm_confobj_new ();

	/* maybe point to the key in the hashtable to save memory? */
	ohm_confobj_set_key (confobj, key);
	ohm_confobj_set_public (confobj, public);
	ohm_confobj_set_value (confobj, value);

	/* we need to create new objects in the store for each added user */

	/* all new keys have to have an added signal */
	ohm_debug ("emit key-added : %s", key);
	g_signal_emit (conf, signals [KEY_ADDED], 0, key, value);

	/* add as the strdup'd value as key is constant */
	confkey = ohm_confobj_get_key (confobj);
	g_hash_table_insert (conf->priv->keys, (gpointer) confkey, (gpointer) confobj);

	return TRUE;
}

/**
 * ohm_conf_set_key:
 *
 * Key has to exist first!
 **/
gboolean
ohm_conf_set_key_internal (OhmConf     *conf,
		           const gchar *key,
		           gint         value,
		           gboolean     internal,
		           GError     **error)
{
	OhmConfObj *confobj;
	gboolean confpublic;
	gint confobjvalue;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (conf->priv->keys == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_INVALID,
				      "Conf invalid");
		return FALSE;
	}

	/* try to find the key in the global conf */
	confobj = g_hash_table_lookup (conf->priv->keys, key);
	if (confobj == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_MISSING,
				      "Key %s missing", key);
		return FALSE;
	}

	/* if we are externally calling this key, check to see if
	   we are allowed to set this key */
	confpublic = ohm_confobj_get_public (confobj);
	if (internal == FALSE && confpublic == FALSE) {
		ohm_debug ("tried to set private key : %s", key);
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_OVERRIDE,
				      "Cannot overwrite private key %s", key);
		return FALSE;
	}

	/* check for the correct type */
	ohm_debug ("set existing key '%s' : %i", key, value);

	/* Only force signal if different */
	confobjvalue = ohm_confobj_get_value (confobj);
	if (confobjvalue != value) {
		ohm_confobj_set_value (confobj, value);
		ohm_debug ("emit key-changed : %s", key);
		g_signal_emit (conf, signals [KEY_CHANGED], 0, key, value);
	}

	return TRUE;
}

/**
 * ohm_conf_process_line:
 **/
static gboolean
ohm_conf_process_line (OhmConf     *conf,
		       const gchar *line,
		       const gchar *plugin_name,
		       GError     **error)
{
	gint len;
	len = strlen (line);
	gchar *out;
	gchar **parts;
	guint i;
	guint value;
	const gchar *key;
	gboolean public;
	gboolean ret;

	/* check we have a long enough string */
	if (len < 2) {
		return TRUE;
	}

	/* check to see if we are a comment */
	if (line[0] == '#') {
		return TRUE;
	}

	/* check for inline comments */
	out = g_strrstr (line, "#");
	if (out != NULL) {
		g_warning ("no inline comments allowed: '%s'", line);
		return FALSE;
	}

	/* check for tabs */
	out = g_strrstr (line, "\t");
	if (out != NULL) {
		g_warning ("no tabs allowed: '%s'", line);
		return FALSE;
	}

	/* split the string by spaces */
	parts = g_strsplit (line, " ", 0);

	/* get the number of space seporated strings */
	i = 0;
	while (parts[i] != NULL) {
		i++;
	}

	/* check we have 2 or 3 parameters */
	if (i != 2 && i != 3) {
		g_warning ("wrong number of parameters: '%s'", line);
		return FALSE;
	}

	/* check the key is prefixed by the plugin name */
	if (g_str_has_prefix  (parts[0], plugin_name) == TRUE) {
		key = parts[0];
		/* do we check if it's allowed to exist? */
	} else {
		g_warning ("can only set %s.* parameters: '%s'", plugin_name, line);
		return FALSE;
	}

	/* check the value is valid */
	value = atoi (parts[1]);

	/* check the 3th parameter is correct */
	if (i != 3) {
		public = FALSE;
	} else {
		if (strcmp (parts[2], "public") == 0) {
			public = TRUE;
		} else {
			g_warning ("Only 'public' valid as 3rd parameter: '%s'", line);
			return FALSE;
		}
	}

	ret = ohm_conf_add_key (conf, key, value, public, error);

	g_strfreev (parts);

	return ret;
}

/**
 * ohm_conf_load_defaults:
 **/
gboolean
ohm_conf_load_defaults (OhmConf     *conf,
			const gchar *plugin_name,
			GError     **error)
{
	gboolean ret;
	gchar *contents;
	gsize length;
	gchar **lines;
	guint i;
	gchar *filename;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (plugin_name != NULL, FALSE);

	/* generate path for each module */
	filename = g_build_path (G_DIR_SEPARATOR_S, SYSCONFDIR, "ohm", "plugins", plugin_name, NULL);

	ohm_debug ("Loading %s defaults from %s", plugin_name, filename);

	/* load from file */
	ret = g_file_get_contents (filename, &contents, &length, error);
	g_free (filename);
	if (ret == FALSE) {
		/* we already set error */
		return FALSE;
	}

	/* split into lines and process each one */
	lines = g_strsplit (contents, "\n", -1);
	i = 0;
	while (lines[i] != NULL) {
		ret = ohm_conf_process_line (conf, lines[i], plugin_name, error);
		if (ret == FALSE) {
			g_error ("Loading keys from conf failed: %s", (*error)->message);
		}
		i++;
	}
	g_strfreev (lines);
	g_free (contents);
	return TRUE;
}

/**
 * ohm_hash_remove_return:
 * FIXME: there must be a better way to do this
 **/
static gboolean
ohm_hash_remove_return (gpointer key,
			gpointer value,
			gpointer user_data)
{
	OhmConfObj *confobj = (OhmConfObj *) value;
	g_object_unref (confobj);
	return TRUE;
}

/**
 * ohm_conf_finalize:
 **/
static void
ohm_conf_finalize (GObject *object)
{
	OhmConf *conf;
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_CONF (object));
	conf = OHM_CONF (object);

	ohm_debug ("freeing conf");
	g_hash_table_foreach_remove (conf->priv->keys,
				     ohm_hash_remove_return, NULL);
	g_hash_table_destroy (conf->priv->keys);
	conf->priv->keys = NULL;

	g_return_if_fail (conf->priv != NULL);
	G_OBJECT_CLASS (ohm_conf_parent_class)->finalize (object);
}

/**
 * ohm_conf_class_init:
 **/
static void
ohm_conf_class_init (OhmConfClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_conf_finalize;

	signals [KEY_ADDED] =
		g_signal_new ("key-added",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmConfClass, key_added),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING_INT,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);
	signals [KEY_CHANGED] =
		g_signal_new ("key-changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmConfClass, key_changed),
			      NULL, NULL,
			      ohm_marshal_VOID__STRING_INT,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);

	g_type_class_add_private (klass, sizeof (OhmConfPrivate));
}

/**
 * ohm_conf_init:
 **/
static void
ohm_conf_init (OhmConf *conf)
{
	conf->priv = OHM_CONF_GET_PRIVATE (conf);
	conf->priv->keys = g_hash_table_new (g_str_hash, g_str_equal);
}

/**
 * ohm_conf_new:
 **/
OhmConf *
ohm_conf_new (void)
{
	static OhmConf *conf = NULL;
	if (conf != NULL) {
		g_object_ref (conf);
	} else {
		conf = g_object_new (OHM_TYPE_CONF, NULL);
	}
	return OHM_CONF (conf);
}
