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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus-glib.h>

#include "libohm.h"
#include "libohm-marshal.h"
#include "../ohmd/ohm-common.h"

static gboolean do_trace = FALSE;

static void trace (const char *format, ...)
{
	va_list args;
	gchar *str;
	FILE *out;

	if (!do_trace)
		return;

	g_return_if_fail (format != NULL);

	va_start(args, format);
	str = g_strdup_vprintf(format, args);
	va_end(args);

	out = stderr;

	fputs("libohm trace: ", out);
	fputs(str, out);

	g_free(str);
}

enum {
	VALUE_CHANGED,
	LAST_SIGNAL
};

static void libohm_class_init (LibOhmClass * klass);
static void libohm_init (LibOhm *ctx);
static void libohm_finalize (GObject *object);

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

GType
libohm_get_type (void)
{
	static GType ctx_type = 0;

	if (!ctx_type) {
		static const GTypeInfo ctx_info = {
			sizeof(LibOhmClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) libohm_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof(LibOhm),
			0,	/* n_preallocs */
			(GInstanceInitFunc) libohm_init
		};

		ctx_type = g_type_register_static(G_TYPE_OBJECT, "LibOhm", &ctx_info, 0);
	}

	return ctx_type;
}

static void
libohm_class_init (LibOhmClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent(class);

	signals[VALUE_CHANGED] =
	    g_signal_new ("value_changed",
			 G_TYPE_FROM_CLASS(object_class),
			 G_SIGNAL_RUN_LAST,
			 G_STRUCT_OFFSET(LibOhmClass, value_changed),
			 NULL, NULL,
			 libohm_marshal_VOID__STRING_INT,
			 G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);

	class->value_changed = NULL;

	object_class->finalize = libohm_finalize;

	if (g_getenv("LIBOHM_DEBUG_TRACE_CTX") != NULL)
		do_trace = TRUE;
}

static void
libohm_init (LibOhm *ctx)
{
	GError *error = NULL;

	ctx->is_initialized = TRUE;
	ctx->is_direct = FALSE;

	gchar *direct_addr;
	direct_addr = getenv ("OHMD_DIRECT_ADDR");
	if (direct_addr != NULL) {
		g_error ("got direct address of %s", direct_addr);
//		DBusGConnection * 	dbus_g_connection_open (const gchar *address, GError **error)
 //	Returns a connection to the given address. 
//dbus_g_proxy_new_for_peer (DBusGConnection *connection, const char *path_name, const char *interface_name)
// 	Creates a proxy for an object in peer application (one we're directly connected to). 
	}

	/* get the DBUS session connection */
	ctx->connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (error != NULL) {
		g_warning ("Unable to get connection : %s", error->message);
		g_error_free (error);
		return;
	}

	/* get the proxy with g-p-m */
	ctx->proxy = dbus_g_proxy_new_for_name (ctx->connection,
					   OHM_DBUS_SERVICE,
					   OHM_DBUS_PATH_KEYSTORE,
					   OHM_DBUS_INTERFACE_KEYSTORE);
	if (ctx->proxy == NULL) {
		g_warning ("Unable to get proxy : %s", OHM_DBUS_INTERFACE_KEYSTORE);
		return;
	}
	trace ("ctx created okay");
}

static void
libohm_finalize (GObject *object)
{
	LibOhm *ctx = LIBOHM_CTX (object);

	g_object_unref (G_OBJECT (ctx->proxy));
	ctx->is_initialized = FALSE;

	if (G_OBJECT_CLASS(parent_class)->finalize)
		(*G_OBJECT_CLASS(parent_class)->finalize) (object);
}

LibOhm *
libohm_new (void)
{
	LibOhm *ctx;

	ctx = g_object_new (libohm_get_type(), NULL);
	g_object_ref (G_OBJECT(ctx));

	return ctx;
}

gboolean
libohm_keystore_get_key (LibOhm		*ctx,
			const gchar	*key,
			gint		*value,
			GError		**error)
{
	gboolean ret;

	g_return_val_if_fail (ctx != NULL, FALSE);
	g_return_val_if_fail (ctx->is_initialized, FALSE);

	ret = dbus_g_proxy_call (ctx->proxy,
				 "GetKey", error,
				 G_TYPE_STRING, key,
				 G_TYPE_INVALID,
				 G_TYPE_INT, value,
				 G_TYPE_INVALID);
	if (ret == FALSE) {
		*value = 0;
	}
	return ret;
}

gboolean
libohm_keystore_set_key (LibOhm		*ctx,
			 const gchar	*key,
			 gint		 value,
			 GError		**error)
{
	gboolean ret;

	g_return_val_if_fail (ctx != NULL, FALSE);
	g_return_val_if_fail (ctx->is_initialized, FALSE);

	ret = dbus_g_proxy_call (ctx->proxy,
				 "SetKey", error,
				 G_TYPE_STRING, key,
				 G_TYPE_INT, value,
				 G_TYPE_INVALID,
				 G_TYPE_INVALID);
	return ret;
}

gboolean
libohm_keystore_add_notify_key (LibOhm		*ctx,
				const gchar	*key,
				GError		**error)
{
	gboolean ret;

	g_return_val_if_fail (ctx != NULL, FALSE);
	g_return_val_if_fail (ctx->is_initialized, FALSE);

	ret = dbus_g_proxy_call (ctx->proxy,
				 "AddNotifyKey", error,
				 G_TYPE_STRING, key,
				 G_TYPE_INVALID,
				 G_TYPE_INVALID);
	return ret;
}

#if 0
void
libohm_value_changed (LibOhm *ctx, const gchar *key, gint value)
{
	g_return_if_fail (ctx != NULL);
	g_return_if_fail (LIBOHM_IS_CTX(ctx));
	g_return_if_fail (key != NULL);

	g_signal_emit (G_OBJECT(ctx), signals[VALUE_CHANGED], 0, key, value);
}
#endif
