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

#ifndef __OHM_KEYSTORE_H
#define __OHM_KEYSTORE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define OHM_TYPE_KEYSTORE		(ohm_keystore_get_type ())
#define OHM_KEYSTORE(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_KEYSTORE, OhmKeystore))
#define OHM_KEYSTORE_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_KEYSTORE, OhmKeystoreClass))
#define OHM_IS_KEYSTORE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_KEYSTORE))
#define OHM_IS_KEYSTORE_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_KEYSTORE))
#define OHM_KEYSTORE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_KEYSTORE, OhmKeystoreClass))

typedef struct OhmKeystorePrivate OhmKeystorePrivate;

typedef struct
{
	GObject		         parent;
	OhmKeystorePrivate *priv;
} OhmKeystore;

typedef struct
{
	GObjectClass	parent_class;
	void		(* key_changed)			(OhmKeystore	*keystore,
							 const gchar	*key);
} OhmKeystoreClass;

typedef enum
{
	 OHM_KEYSTORE_ERROR_INVALID,
	 OHM_KEYSTORE_ERROR_KEY_MISSING,
	 OHM_KEYSTORE_ERROR_KEY_OVERRIDE,
	 OHM_KEYSTORE_ERROR_KEY_LAST
} OhmKeystoreError;

GType		 ohm_keystore_get_type			(void);
GQuark		 ohm_keystore_error_quark		(void);
OhmKeystore 	*ohm_keystore_new			(void);

gboolean	 ohm_keystore_get_key			(OhmKeystore	*keystore,
							 const gchar	*key,
							 gint		*value,
							 GError		**error);
gboolean	 ohm_keystore_set_key			(OhmKeystore	*keystore,
							 const gchar	*key,
							 gint		 value,
							 GError		**error);
gboolean	 ohm_keystore_add_notify_key		(OhmKeystore	*keystore,
							 const gchar	*key,
							 GError		**error);
gboolean	 ohm_keystore_get_keys			(OhmKeystore	*keystore,
							 GPtrArray	**data,
							 GError		**error);

G_END_DECLS

#endif /* __OHM_KEYSTORE_H */
