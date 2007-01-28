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
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ohm-debug.h"
#include "ohm-common.h"
#include "ohm-manager.h"
#include "ohm-conf.h"
#include "ohm-keystore.h"
#include "ohm-module.h"
#include "ohm-dbus-keystore.h"

static void     ohm_manager_class_init	(OhmManagerClass *klass);
static void     ohm_manager_init	(OhmManager      *manager);
static void     ohm_manager_finalize	(GObject	 *object);

#define OHM_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MANAGER, OhmManagerPrivate))

struct OhmManagerPrivate
{
	OhmConf			*conf;
	OhmModule		*module;
	OhmKeystore		*keystore;
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

/**
 * ohm_manager_class_init:
 * @klass: The OhmManagerClass
 **/
static void
ohm_manager_class_init (OhmManagerClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	   = ohm_manager_finalize;

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
	GError *error = NULL;
	DBusGConnection *connection;
	manager->priv = OHM_MANAGER_GET_PRIVATE (manager);

	/* get system bus connection */
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (error != NULL) {
		g_error ("Cannot get connection to system bus!");
		g_error_free (error);
	}

	manager->priv->conf = ohm_conf_new ();
	manager->priv->module = ohm_module_new ();

	/* add the keystore and the DBUS interface */
	manager->priv->keystore = ohm_keystore_new ();
	dbus_g_object_type_install_info (OHM_TYPE_KEYSTORE, &dbus_glib_ohm_keystore_object_info);
	dbus_g_connection_register_g_object (connection, OHM_DBUS_PATH_KEYSTORE, G_OBJECT (manager->priv->keystore));

	/* set some predefined keys */
	ohm_conf_add_key (manager->priv->conf, "manager.version.major", VERSION_MAJOR, FALSE, &error);
	ohm_conf_add_key (manager->priv->conf, "manager.version.minor", VERSION_MINOR, FALSE, &error);
	ohm_conf_add_key (manager->priv->conf, "manager.version.patch", VERSION_PATCH, FALSE, &error);

	/* list the users, should only be root at startup */
	ohm_conf_user_list (manager->priv->conf);
}

/**
 * ohm_manager_finalize:
 * @object: The object to finalize
 *
 * Finalise the manager, by unref'ing all the depending modules.
 **/
static void
ohm_manager_finalize (GObject *object)
{
	OhmManager *manager;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MANAGER (object));
	manager = OHM_MANAGER (object);
	g_return_if_fail (manager->priv != NULL);

	g_object_unref (manager->priv->module);
	g_object_unref (manager->priv->keystore);
	g_object_unref (manager->priv->conf);

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

	return OHM_MANAGER (manager);
}
