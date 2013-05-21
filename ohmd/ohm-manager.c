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
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ohm-debug.h"
#include "ohm-common.h"
#include "ohm-manager.h"
#include "ohm-conf.h"
#ifdef HAVE_KEYSTORE
#include "ohm-keystore.h"
#include "ohm-dbus-keystore.h"
#endif
#include "ohm-module.h"


static void     ohm_manager_class_init	(OhmManagerClass *klass);
static void     ohm_manager_init	(OhmManager      *manager);
static void     ohm_manager_dispose	(GObject	 *object);
static void     ohm_manager_finalize	(GObject	 *object);

#define OHM_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MANAGER, OhmManagerPrivate))

struct OhmManagerPrivate
{
	OhmConf			*conf;
	OhmModule		*module;
#ifdef HAVE_KEYSTORE
	OhmKeystore		*keystore;
#else
        void                    *no_keystore;
#endif
        GKeyFile                *options;
};

enum {
	ON_AC_CHANGED,
	LAST_SIGNAL
};

static guint	     signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (OhmManager, ohm_manager, G_TYPE_OBJECT)

/**
 * ohm_manager_error_quark:
 * Return value: Our personal error quark.
 **/
GQuark
ohm_manager_error_quark (void)
{
	static GQuark quark = 0;
	if (!quark) {
		quark = g_quark_from_static_string ("ohm_manager_error");
	}
	return quark;
}

/**
 * ohm_manager_get_version:
 **/
gboolean
ohm_manager_get_version (OhmManager *manager,
		         gchar     **version,
		         GError    **error)
{
	g_return_val_if_fail (OHM_IS_MANAGER (manager), FALSE);

	if (version == NULL) {
		return FALSE;
	}

	ohm_debug ("creating version string");

	/* return a x.y.z version string */
	*version = g_strdup_printf ("%i.%i.%i", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	return TRUE;
}

/**
 * ohm_manager_get_plugins:
 **/
gboolean
ohm_manager_get_plugins (OhmManager  *manager,
			 gchar     ***plugins,
			 GError     **error)
{
	g_return_val_if_fail (OHM_IS_MANAGER (manager), FALSE);

	if (plugins == NULL) {
		return FALSE;
	}

	*plugins = NULL;

	return TRUE;
}


static void
ohm_manager_load_options(OhmManager *manager)
{
	char    path[PATH_MAX], *dir;
	GError *error;
	
	if ((dir = getenv("OHM_CONF_DIR")) != NULL)
		snprintf(path, sizeof(path), "%s%sohmd.ini",
			 dir, G_DIR_SEPARATOR_S);
	else
		snprintf(path, sizeof(path), "%s%s%s%sohmd.ini",
			 SYSCONFDIR, G_DIR_SEPARATOR_S,
			 "ohm", G_DIR_SEPARATOR_S);
	
	manager->priv->options = g_key_file_new();
	
	error = NULL;
	if (!g_key_file_load_from_file(manager->priv->options, path,
				       G_KEY_FILE_NONE, &error) && 
	    error && error->code != G_KEY_FILE_ERROR_NOT_FOUND)
		g_warning("Failed to load config file \"%s\".", path);
}


static void
ohm_manager_free_options(OhmManager *manager)
{
	if (manager->priv->options != NULL)
		g_key_file_free(manager->priv->options);
	
	manager->priv->options = NULL;
}


/**
 * ohm_manager_get_boolean_option:
 */
gboolean
ohm_manager_get_boolean_option(OhmManager *manager,
			       const gchar *group, const gchar *key)
{
  	GError *error;

  	if (manager->priv->options == NULL)
  		return FALSE;
      
	error = NULL;
	return g_key_file_get_boolean(manager->priv->options,
				      group ? group : "global", key, &error);
}


/**
 * ohm_manager_get_string_option:
 */
gchar *
ohm_manager_get_string_option(OhmManager *manager,
			      const gchar *group, const gchar *key)
{
  	GError *error;

	if (manager->priv->options == NULL)
		return NULL;
      
	error = NULL;
	return g_key_file_get_string(manager->priv->options,
				     group ? group : "global", key, &error);
}


/**
 * ohm_manager_get_integer_option:
 */
gint
ohm_manager_get_integer_option(OhmManager *manager,
			       const gchar *group, const gchar *key)
{
	GError *error;

	if (manager->priv->options == NULL)
		return 0;
      
	error = NULL;
	return g_key_file_get_integer(manager->priv->options,
				      group ? group : "global", key, &error);
}


/**
 * ohm_manager_get_double_option:
 */
gdouble
ohm_manager_get_double_option(OhmManager *manager,
			      const gchar *group, const gchar *key)
{
	GError *error;

	if (manager->priv->options == NULL)
		return 0.0;
      
	error = NULL;
	return g_key_file_get_double(manager->priv->options,
				     group ? group : "global", key, &error);
}


/**
 * ohm_manager_class_init:
 * @klass: The OhmManagerClass
 **/
static void
ohm_manager_class_init (OhmManagerClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	= ohm_manager_finalize;
	object_class->dispose	= ohm_manager_dispose;

	signals [ON_AC_CHANGED] =
		g_signal_new ("on-ac-changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmManagerClass, on_ac_changed),
			      NULL,
			      NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1, G_TYPE_BOOLEAN);

	g_type_class_add_private (klass, sizeof (OhmManagerPrivate));
}

/**
 * ohm_manager_init:
 **/
static void
ohm_manager_init (OhmManager *manager)
{
#ifdef HAVE_KEYSTORE
	GError *error = NULL;
	DBusGConnection *connection;
#endif
	manager->priv = OHM_MANAGER_GET_PRIVATE (manager);

#ifdef HAVE_KEYSTORE
	/* get system bus connection */
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (error != NULL) {
		g_error ("Cannot get connection to system bus!");
		g_error_free (error);
	}
#endif

	ohm_manager_load_options(manager);

	manager->priv->conf = ohm_conf_new ();
	manager->priv->module = ohm_module_new ();

#ifdef HAVE_KEYSTORE
	/* add the keystore and the DBUS interface */
	manager->priv->keystore = ohm_keystore_new ();
	dbus_g_object_type_install_info (OHM_TYPE_KEYSTORE, &dbus_glib_ohm_keystore_object_info);
	dbus_g_connection_register_g_object (connection, OHM_DBUS_PATH_KEYSTORE, G_OBJECT (manager->priv->keystore));

	/* set some predefined keys */
	ohm_conf_add_key (manager->priv->conf, "manager.version.major", VERSION_MAJOR, FALSE, &error);
	ohm_conf_add_key (manager->priv->conf, "manager.version.minor", VERSION_MINOR, FALSE, &error);
	ohm_conf_add_key (manager->priv->conf, "manager.version.patch", VERSION_PATCH, FALSE, &error);
#endif
}

/**
 * ohm_manager_dispose:
 * @object: The object to dispose
 *
 * unref all objects we're holding
 **/
static void
ohm_manager_dispose (GObject *object)
{
	OhmManager *manager;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MANAGER (object));
	manager = OHM_MANAGER (object);
	g_return_if_fail (manager->priv != NULL);

	g_object_unref (manager->priv->module);
#ifdef HAVE_KEYSTORE
	g_object_unref (manager->priv->keystore);
#endif
	g_object_unref (manager->priv->conf);

	ohm_manager_free_options(manager);

	G_OBJECT_CLASS (ohm_manager_parent_class)->dispose (object);
}

/**
 * ohm_manager_finalize:
 * @object: The object to finalize
 *
 * Finalise the manager
 **/
static void
ohm_manager_finalize (GObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MANAGER (object));

	g_debug ("Finalizing ohm_manager");
	G_OBJECT_CLASS (ohm_manager_parent_class)->finalize (object);
}

/**
 * ohm_manager_new:
 *
 * Return value: a new OhmManager object.
 **/
OhmManager *
ohm_manager_new (void)
{
	OhmManager *manager;

	manager = g_object_new (OHM_TYPE_MANAGER, NULL);

	manager = OHM_MANAGER (manager);
	return manager;
}
