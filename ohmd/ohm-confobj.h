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

#ifndef __OHM_CONFOBJ_H
#define __OHM_CONFOBJ_H

#include <glib-object.h>
#include <ohm/ohm-plugin.h>

G_BEGIN_DECLS

#define OHM_TYPE_CONFOBJ		(ohm_confobj_get_type ())
#define OHM_CONFOBJ(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_CONFOBJ, OhmConfObj))
#define OHM_CONFOBJ_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_CONFOBJ, OhmConfObjClass))
#define OHM_IS_CONFOBJ(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_CONFOBJ))
#define OHM_IS_CONFOBJ_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_CONFOBJ))
#define OHM_CONFOBJ_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_CONFOBJ, OhmConfObjClass))

typedef struct OhmConfObjPrivate OhmConfObjPrivate;

typedef struct
{
	GObject		         parent;
	OhmConfObjPrivate *priv;
} OhmConfObj;

typedef struct
{
	GObjectClass	parent_class;
} OhmConfObjClass;

GType		 ohm_confobj_get_type			(void);
OhmConfObj 	*ohm_confobj_new			(void);

gchar		*ohm_confobj_get_key			(OhmConfObj *confobj);
gboolean	 ohm_confobj_set_key			(OhmConfObj *confobj, const gchar *value);
gint		 ohm_confobj_get_value			(OhmConfObj *confobj);
gboolean	 ohm_confobj_set_value			(OhmConfObj *confobj, gint value);
gboolean	 ohm_confobj_get_public			(OhmConfObj *confobj);
void		 ohm_confobj_set_public			(OhmConfObj *confobj, gboolean public);

G_END_DECLS

#endif /* __OHM_CONFOBJ_H */
