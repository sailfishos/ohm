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

#ifndef __OHM_MANAGER_H
#define __OHM_MANAGER_H

#include <glib-object.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#define OHM_TYPE_MANAGER	 (ohm_manager_get_type ())
#define OHM_MANAGER(o)		 (G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_MANAGER, OhmManager))
#define OHM_MANAGER_CLASS(k)	 (G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_MANAGER, OhmManagerClass))
#define OHM_IS_MANAGER(o)	 (G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_MANAGER))
#define OHM_IS_MANAGER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_MANAGER))
#define OHM_MANAGER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_MANAGER, OhmManagerClass))

typedef struct OhmManagerPrivate OhmManagerPrivate;

typedef struct
{
	 GObject		 parent;
	 OhmManagerPrivate	*priv;
} OhmManager;

typedef struct
{
	GObjectClass	parent_class;
	void		(* on_ac_changed)		(OhmManager	*manager,
							 gboolean	 on_ac);
} OhmManagerClass;

typedef enum
{
	 OHM_MANAGER_ERROR_GENERAL,
	 OHM_MANAGER_ERROR_LAST
} OhmManagerError;

#define OHM_MANAGER_ERROR ohm_manager_error_quark ()

GQuark		 ohm_manager_error_quark		(void);
GType		 ohm_manager_get_type		  	(void);
OhmManager	*ohm_manager_new			(void);

gboolean	 ohm_manager_get_version		(OhmManager	*manager,
							 gchar		**version,
							 GError		**error);
gboolean	 ohm_manager_get_plugins		(OhmManager	*manager,
							 gchar		***retval,
							 GError		**error);

G_END_DECLS

#endif /* __OHM_MANAGER_H */
