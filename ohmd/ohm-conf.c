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
#include "ohm-marshal.h"

#define OHM_CONF_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_CONF, OhmConfPrivate))

typedef struct {
	gchar			*name;
	guint			 uid;
} OhmConfUser;

typedef struct {
	gchar			*key;
	gboolean		 public;
	gint			*current;
	GPtrArray		*store; /* gint object are stored for each user if public */
} OhmConfObjectMulti;

struct OhmConfPrivate
{
	GHashTable		*keys;
	GPtrArray		*users;
	OhmConfUser		*current_user;
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
 * ohm_conf_user_list:
 **/
gboolean
ohm_conf_user_list (OhmConf *conf)
{
	gint i;
	OhmConfUser *confuser;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);

	g_print ("Printing user list:\n");
	for (i=0; i < conf->priv->users->len; i++) {
		confuser = (OhmConfUser *) g_ptr_array_index (conf->priv->users, i);
		g_print ("number:%i\tuid:%i\t%s\n", i, confuser->uid, confuser->name);
	}

	return TRUE;
}

/**
 * ohm_conf_user_obj_from_name:
 *
 * returns NULL if not found
 **/
static OhmConfUser *
ohm_conf_user_obj_from_name (OhmConf     *conf,
		             const gchar *user)
{
	gint i;
	OhmConfUser *confuser;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);

	for (i=0; i < conf->priv->users->len; i++) {
		confuser = (OhmConfUser *) g_ptr_array_index (conf->priv->users, i);
		if (strcmp (confuser->name, user) == 0) {
			return confuser;
		}
	}

	return NULL;
}

/**
 * ohm_conf_user_switch:
 *
 * user "root" for default
 **/
gboolean
ohm_conf_user_switch (OhmConf     *conf,
		      const gchar *user,
		      GError     **error)
{
	OhmConfUser *confuser;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (user != NULL, FALSE);

	ohm_debug ("Trying to switch user to %s", user);
	confuser = ohm_conf_user_obj_from_name (conf, user);

	/* cannot find the user */
	if (confuser == NULL) {
		ohm_debug ("Cannot find user '%s'", user);
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_USER_INVALID,
				      "user not found");
		return FALSE;
	}

	/* switch the current pointers to this uid */
	ohm_debug ("switching to uid %i, name %s", confuser->uid, confuser->name);
	conf->priv->current_user = confuser;
	/* FIXME, switch pointers for all public values */

	return TRUE;
}

/**
 * ohm_conf_user_add:
 *
 * Does not switch to the new user, use ohm_conf_user_switch() for this
 **/
gboolean
ohm_conf_user_add (OhmConf     *conf,
		   const gchar *user,
		   GError     **error)
{
	OhmConfUser *confuser;
	static gint uid_global = 0;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (user != NULL, FALSE);

	ohm_debug ("Trying to add user %s.", user);

	/* search for existing user */
	confuser = ohm_conf_user_obj_from_name (conf, user);
	if (confuser != NULL) {
		/* we found the user in the list */
		ohm_debug ("Cannot add user '%s' as already in database.", user);
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_USER_INVALID,
				      "user already added");
		return FALSE;
	}

	/* allocate a new user with a unique uid */
	confuser = g_new (OhmConfUser, 1);
	confuser->uid = uid_global++;
	confuser->name = g_strdup (user);

	/* add the user */
	g_ptr_array_add (conf->priv->users, (gpointer) confuser);

	return TRUE;
}

/**
 * ohm_conf_user_remove:
 **/
gboolean
ohm_conf_user_remove (OhmConf     *conf,
		      const gchar *user,
		      GError     **error)
{
	OhmConfUser *confuser;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (user != NULL, FALSE);

	ohm_debug ("Trying to remove user %s.", user);

	confuser = ohm_conf_user_obj_from_name (conf, user);
	if (confuser == NULL) {
		ohm_debug ("Cannot remove user '%s' as does not exist in database", user);
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_USER_INVALID,
				      "user not found");
		return FALSE;
	}

	/* remove from the userlist */
	g_ptr_array_remove (conf->priv->users, (gpointer) confuser);
	g_free (confuser->name);
	g_free (confuser);

	/* make sure we are not leaving the current user settings dangling */
	if (conf->priv->current_user == confuser) {
		ohm_conf_user_switch (conf, "root", NULL);
	}

	return TRUE;
}

/**
 * ohm_conf_compare_func
 * A GCompareFunc for comparing two OhmConfObjectMulti objects by key.
 **/
static gint
ohm_conf_compare_func (gconstpointer a, gconstpointer b)
{
	OhmConfObjectMulti *entry1 = (OhmConfObjectMulti *) a;
	OhmConfObjectMulti *entry2 = (OhmConfObjectMulti *) b;
	return strcmp (entry1->key, entry2->key);
}

/**
 * ohm_conf_print_all_iter:
 **/
static void
ohm_conf_to_slist_iter (gpointer	     key,
			OhmConfObjectMulti *entry,
			gpointer	    *user_data)
{
	GSList **list = (GSList **) user_data;
	*list = g_slist_insert_sorted (*list, (gpointer) entry, ohm_conf_compare_func);
}

/**
 * ohm_conf_print_all:
 * @conf: This class instance
 *
 * Prints an ordered pretty print of the conf.
 * This is quite slow, so avoid putting in production code.
 **/
gboolean
ohm_conf_print_all (OhmConf *conf)
{
	GSList *list = NULL;
	GSList *l;
	guint max = 0;
	guint len;
	gchar *spaces;
	OhmConfObjectMulti *entry;

	g_print ("All keys and values in database:\n");
	if (conf->priv->keys == NULL) {
		g_warning ("Conf invalid");
		return FALSE;
	}

	/* add hash to sorted SList */
	g_hash_table_foreach (conf->priv->keys, (GHFunc) ohm_conf_to_slist_iter, &list);

	/* get max length of keys */
	for (l=list; l != NULL; l=l->next) {
		entry = (OhmConfObjectMulti *) l->data;
		len = strlen (entry->key);
		if (len > max) {
			max = len;
		}
	}

	/* print the keys */
	for (l=list; l != NULL; l=l->next) {
		entry = (OhmConfObjectMulti *) l->data;
		g_print ("%s", entry->key);
		spaces = g_strnfill (max - strlen(entry->key), ' ');
		if (entry->public == TRUE) {
			g_print ("%s : %i\t(public)\n", spaces, *(entry->current));
		} else {
			g_print ("%s : %i\t(private)\n", spaces, *(entry->current));
		}
		g_free (spaces);
	}

	/* free the list */
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
	OhmConfObjectMulti *entry;
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
	entry = g_hash_table_lookup (conf->priv->keys, key);
	if (entry == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_MISSING,
				      "Key missing");
		*value = 0;
		return FALSE;
	}

	/* copy value from key */
	*value = *(entry->current);
	return TRUE;
}

/**
 * ohm_conf_set_key:
 * internal set true for plugin access and false for public dbus
 *
 **/
gboolean
ohm_conf_set_key_internal (OhmConf     *conf,
		           const gchar *key,
		           gint         value,
		           gboolean     internal,
		           GError     **error)
{
	OhmConfObjectMulti *entry;
	gint *intobj;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (conf->priv->keys == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_INVALID,
				      "Conf invalid");
		return FALSE;
	}

	/* try to find the key in the global conf */
	entry = g_hash_table_lookup (conf->priv->keys, key);
	if (entry == NULL) {
		/* create a new key */
		ohm_debug ("create key '%s' : %i", key, value);
		entry = g_new0 (OhmConfObjectMulti, 1);

		/* maybe point to the key in the hashtable to save memory? */
		entry->key = g_strdup (key);
		entry->public = FALSE;
		entry->store = g_ptr_array_new ();

		/* we always create one int object for the private value */
		intobj = g_new0 (gint, 1);
		g_ptr_array_add (entry->store, (gpointer) intobj);

		/* assume the setting user is the current user */
		entry->current = intobj;
		*(entry->current) = value;

		/* we need to create new objects in the store for each added user */

		/* all new keys have to have an added signal */
		ohm_debug ("emit key-added : %s", key);
		g_signal_emit (conf, signals [KEY_ADDED], 0, key, value);

		g_hash_table_insert (conf->priv->keys, entry->key, entry);
	} else {
		/* if we are externally calling this key, check to see if
		   we are allowed to set this key */
		if (internal == FALSE && entry->public == FALSE) {
			ohm_debug ("tried to overwrite private key : %s", key);
			*error = g_error_new (ohm_conf_error_quark (),
					      OHM_CONF_ERROR_KEY_OVERRIDE,
					      "Key cannot overwrite private key");
			return FALSE;
		}

		/* check for the correct type */
		ohm_debug ("overwrite key '%s' : %i", key, value);

		/* Only force signal if different */
		if (*(entry->current) != value) {
			*(entry->current) = value;
			ohm_debug ("emit key-changed : %s", key);
			g_signal_emit (conf, signals [KEY_CHANGED], 0, key, value);
		}
	}

	return TRUE;
}

/**
 * ohm_conf_set_public:
 *
 **/
static gboolean
ohm_conf_set_public (OhmConf     *conf,
		     const gchar *key,
		     gboolean     public,
		     GError     **error)
{
	OhmConfObjectMulti *entry;

	g_return_val_if_fail (OHM_IS_CONF (conf), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (conf->priv->keys == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_INVALID,
				      "Conf invalid");
		return FALSE;
	}

	/* try to find the key in the global conf */
	entry = g_hash_table_lookup (conf->priv->keys, key);
	if (entry == NULL) {
		*error = g_error_new (ohm_conf_error_quark (),
				      OHM_CONF_ERROR_KEY_MISSING,
				      "Key missing");
		return FALSE;
	}

	/* set the public bit */
	entry->public = public;

	/* FIXME: do something clever with *current */

	return TRUE;
}

/**
 * ohm_conf_process_line:
 **/
static gboolean
ohm_conf_process_line (OhmConf     *conf,
		       const gchar *line,
		       const gchar *plugin_name)
{
	gint len;
	len = strlen (line);
	gchar *out;
	gchar **parts;
	guint i;
	guint value;
	const gchar *key;
	gboolean public;

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

	ohm_conf_set_key_internal (conf, key, value, TRUE, NULL);
	ohm_conf_set_public (conf, key, public, NULL);

	g_strfreev (parts);

	return TRUE;
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
		ret = ohm_conf_process_line (conf, lines[i], plugin_name);
		if (ret == FALSE) {
			g_error ("Loading keys from conf failed");
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
	OhmConfObjectMulti *entry = (OhmConfObjectMulti *) value;
	g_free (entry->key);
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
	conf->priv->users = g_ptr_array_new ();
	conf->priv->current_user = NULL;
	/* add root user */
	ohm_conf_user_add (conf, "root", NULL);
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
