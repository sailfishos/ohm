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

#ifndef __OHM_CONF_H
#define __OHM_CONF_H

#include <glib-object.h>

G_BEGIN_DECLS

#define OHM_TYPE_CONF		(ohm_conf_get_type ())
#define OHM_CONF(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_CONF, OhmConf))
#define OHM_CONF_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_CONF, OhmConfClass))
#define OHM_IS_CONF(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_CONF))
#define OHM_IS_CONF_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_CONF))
#define OHM_CONF_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_CONF, OhmConfClass))

typedef struct OhmConfPrivate OhmConfPrivate;

typedef struct
{
	GObject		         parent;
	OhmConfPrivate *priv;
} OhmConf;

typedef struct
{
	GObjectClass	parent_class;
	void		(* key_added)			(OhmConf	*conf,
							 const gchar	*key,
							 gint		 value);
	void		(* key_changed)			(OhmConf	*conf,
							 const gchar	*key,
							 gint		 value);
} OhmConfClass;

/* ABI stable representation suitable for consumption by session apps */
typedef struct {
	gchar		*name;
	gint		 value;
	gboolean	 public;
} OhmConfKeyValue;

typedef enum
{
	 OHM_CONF_ERROR_INVALID,
	 OHM_CONF_ERROR_KEY_MISSING,
	 OHM_CONF_ERROR_KEY_ALREADY_EXISTS,
	 OHM_CONF_ERROR_KEY_OVERRIDE,
	 OHM_CONF_ERROR_USER_INVALID,
	 OHM_CONF_ERROR_KEY_LAST
} OhmConfError;

GType		 ohm_conf_get_type			(void);
GQuark		 ohm_conf_error_quark			(void);
OhmConf 	*ohm_conf_new				(void);

gboolean	 ohm_conf_get_keys			(OhmConf	*conf,
							 GSList		**list);
gboolean	 ohm_conf_get_key			(OhmConf	*conf,
							 const gchar	*key,
							 gint		*value,
							 GError		**error);
gboolean	 ohm_conf_set_key_internal		(OhmConf	*conf,
							 const gchar	*key,
							 gint		 value,
							 gboolean	 internal,
							 GError		**error);
gboolean	 ohm_conf_add_key			(OhmConf	*conf,
							 const gchar	*key,
							 gint		 value,
							 gboolean	 public,
							 GError		**error);
gboolean	 ohm_conf_load_defaults			(OhmConf	*conf,
							 const gchar	*pluginname,
							 GError		**error);
G_END_DECLS

#endif /* __OHM_CONF_H */
