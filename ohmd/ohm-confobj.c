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

#include <glib/gi18n.h>

#include "ohm-debug.h"
#include "ohm-confobj.h"

#define OHM_CONFOBJ_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_CONFOBJ, OhmConfObjPrivate))

struct OhmConfObjPrivate
{
	gchar			*key;
	gboolean		 public;
	gint			 value;
};

G_DEFINE_TYPE (OhmConfObj, ohm_confobj, G_TYPE_OBJECT)

/**
 * ohm_confobj_get_key:
 **/
gchar *
ohm_confobj_get_key (OhmConfObj *confobj)
{
	g_return_val_if_fail (OHM_IS_CONFOBJ (confobj), NULL);

	return confobj->priv->key;
}

/**
 * ohm_confobj_set_key:
 **/
gboolean
ohm_confobj_set_key (OhmConfObj *confobj, const gchar *value)
{
	g_return_val_if_fail (OHM_IS_CONFOBJ (confobj), FALSE);

	confobj->priv->key = g_strdup (value);

	return TRUE;
}

/**
 * ohm_confobj_get_value:
 **/
gint
ohm_confobj_get_value (OhmConfObj *confobj)
{
	g_return_val_if_fail (OHM_IS_CONFOBJ (confobj), -1);
	return confobj->priv->value;
}

/**
 * ohm_confobj_set_value:
 **/
gboolean
ohm_confobj_set_value (OhmConfObj *confobj, gint value)
{
	g_return_val_if_fail (OHM_IS_CONFOBJ (confobj), FALSE);
	confobj->priv->value = value;
	return TRUE;
}

/**
 * ohm_confobj_get_public:
 **/
gboolean
ohm_confobj_get_public (OhmConfObj *confobj)
{
	g_return_val_if_fail (OHM_IS_CONFOBJ (confobj), FALSE);

	return confobj->priv->public;
}

/**
 * ohm_confobj_set_public:
 **/
void
ohm_confobj_set_public (OhmConfObj *confobj, gboolean public)
{
	g_return_if_fail (OHM_IS_CONFOBJ (confobj));

	confobj->priv->public = public;
}

/**
 * ohm_confobj_finalize:
 **/
static void
ohm_confobj_finalize (GObject *object)
{
	OhmConfObj *confobj;

	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_CONFOBJ (object));
	confobj = OHM_CONFOBJ (object);

	if (confobj->priv->key != NULL) {
		g_free (confobj->priv->key);
	}

	g_return_if_fail (confobj->priv != NULL);
	G_OBJECT_CLASS (ohm_confobj_parent_class)->finalize (object);
}

/**
 * ohm_confobj_class_init:
 **/
static void
ohm_confobj_class_init (OhmConfObjClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_confobj_finalize;

	g_type_class_add_private (klass, sizeof (OhmConfObjPrivate));
}

/**
 * ohm_confobj_init:
 **/
static void
ohm_confobj_init (OhmConfObj *confobj)
{
	confobj->priv = OHM_CONFOBJ_GET_PRIVATE (confobj);
	confobj->priv->key = NULL;
	confobj->priv->public = FALSE;
	confobj->priv->value = 0;
}

/**
 * ohm_confobj_new:
 **/
OhmConfObj *
ohm_confobj_new (void)
{
	OhmConfObj *confobj;
	confobj = g_object_new (OHM_TYPE_CONFOBJ, NULL);
	return OHM_CONFOBJ (confobj);
}
