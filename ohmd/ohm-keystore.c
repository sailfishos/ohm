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

#include "ohm-keystore.h"
#include "ohm-conf.h"

#define OHM_KEYSTORE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_KEYSTORE, OhmKeystorePrivate))

struct OhmKeystorePrivate
{
	OhmConf			*conf;
};

enum {
	KEY_CHANGED,
	LAST_SIGNAL
};

static guint	     signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (OhmKeystore, ohm_keystore, G_TYPE_OBJECT)

/**
 * ohm_keystore_get_key:
 **/
gboolean
ohm_keystore_get_key (OhmKeystore *keystore,
		      const gchar *key,
		      gint        *value,
		      GError     **error)
{
	return ohm_conf_get_key (keystore->priv->conf, key, value, error);
}

/**
 * ohm_keystore_add_notify_key:
 **/
gboolean
ohm_keystore_add_notify_key (OhmKeystore *keystore,
		             const gchar *key,
		             GError     **error)
{
	/* do this internally to this module with a hashtable! */
	//return ohm_conf_add_notify_key (keystore->priv->conf, key, error);
	return FALSE;
}

/**
 * ohm_keystore_set_key:
 *
 **/
gboolean
ohm_keystore_set_key (OhmKeystore *keystore,
		      const gchar *key,
		      gint         value,
		      GError     **error)
{
	return ohm_conf_set_key_internal (keystore->priv->conf, key, value, FALSE, error);
}

/**
 * ohm_keystore_finalize:
 **/
static void
ohm_keystore_finalize (GObject *object)
{
	OhmKeystore *keystore;
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_KEYSTORE (object));
	keystore = OHM_KEYSTORE (object);

	g_object_unref (keystore->priv->conf);

	g_return_if_fail (keystore->priv != NULL);
	G_OBJECT_CLASS (ohm_keystore_parent_class)->finalize (object);
}

/**
 * ohm_keystore_class_init:
 **/
static void
ohm_keystore_class_init (OhmKeystoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_keystore_finalize;

	signals [KEY_CHANGED] =
		g_signal_new ("key-changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (OhmKeystoreClass, key_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 1, G_TYPE_STRING);

	g_type_class_add_private (klass, sizeof (OhmKeystorePrivate));
}

/**
 * ohm_keystore_init:
 **/
static void
ohm_keystore_init (OhmKeystore *keystore)
{
	keystore->priv = OHM_KEYSTORE_GET_PRIVATE (keystore);
	keystore->priv->conf = ohm_conf_new ();
}

/**
 * ohm_keystore_new:
 **/
OhmKeystore *
ohm_keystore_new (void)
{
	OhmKeystore *keystore;
	keystore = g_object_new (OHM_TYPE_KEYSTORE, NULL);
	return OHM_KEYSTORE (keystore);
}
