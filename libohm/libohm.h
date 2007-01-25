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

#ifndef LIBOHM_H
#define LIBOHM_H

#include <glib-object.h>
#include <dbus/dbus-glib.h>

typedef struct _LibOhm LibOhm;
typedef struct _LibOhmClass LibOhmClass;

#define LIBOHM_GET_TYPE			(libohm_get_type ())
#define LIBOHM_CTX(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), LIBOHM_GET_TYPE, LibOhm))
#define LIBOHM_CTX_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), LIBOHM_GET_TYPE, LibOhmClass))
#define LIBOHM_IS_CTX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), LIBOHM_GET_TYPE))
#define LIBOHM_IS_CTX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), LIBOHM_GET_TYPE))
#define LIBOHM_CTX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), LIBOHM_GET_TYPE, LibOhmClass))

struct _LibOhm {
	GObject object;
	/*< private > */
	gboolean	 is_initialized;
	DBusGProxy      *proxy;
	DBusGConnection *connection;
	gpointer	 pad1;
	guint		 pad2;
};

struct _LibOhmClass {
	GObjectClass parent_class;
	void (*value_changed) (LibOhm *ctx, const gchar *key, gint value);
	GFunc		 pad1;
	GFunc		 pad2;
	GFunc		 pad3;
};

typedef struct {
	gchar		*name;
	gint		 value;
	gboolean	 public;
} LibOhmKeyValue;

GType		 libohm_get_type		(void);
LibOhm		*libohm_new			(void);
gboolean	 libohm_keystore_get_key	(LibOhm		*ctx,
						 const gchar	*key,
						 gint		*value,
						 GError		**error);
gboolean	 libohm_keystore_set_key	(LibOhm		*ctx,
						 const gchar	*key,
						 gint		 value,
						 GError		**error);
gboolean	 libohm_keystore_add_notify_key (LibOhm		*ctx,
						 const gchar	*key,
						 GError		**error);
gboolean	 libohm_keystore_get_keys	(LibOhm		*ctx,
						 GSList		**list,
						 GError		**error);
gboolean	 libohm_keystore_free_keys	(LibOhm		*ctx,
						 GSList		*list);

#endif /* LIBOHM_H */
