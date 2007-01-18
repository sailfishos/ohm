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

#ifndef __OHM_MODULE_H
#define __OHM_MODULE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define OHM_TYPE_MODULE		(ohm_module_get_type ())
#define OHM_MODULE(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_MODULE, OhmModule))
#define OHM_MODULE_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_MODULE, OhmModuleClass))
#define OHM_IS_MODULE(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_MODULE))
#define OHM_IS_MODULE_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_MODULE))
#define OHM_MODULE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_MODULE, OhmModuleClass))

typedef struct OhmModulePrivate OhmModulePrivate;

typedef struct
{
	GObject		         parent;
	OhmModulePrivate *priv;
} OhmModule;

typedef struct
{
	GObjectClass	parent_class;
} OhmModuleClass;

GType		 ohm_module_get_type			(void);
OhmModule 	*ohm_module_new				(void);

gboolean	 ohm_module_require			(OhmModule	*module,
							 const gchar	*name);
gboolean	 ohm_module_suggest			(OhmModule	*module,
							 const gchar	*name);
gboolean	 ohm_module_prevent			(OhmModule	*module,
							 const gchar	*name);

G_END_DECLS

#endif /* __OHM_MODULE_H */
