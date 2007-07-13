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
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <glib.h>

#include <libohm.h>

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	GError *error;
	GOptionContext *context;
	gboolean ret;
	LibOhm *ctx;
	gchar *key = NULL;
	gint value = 0;

	const GOptionEntry options[] = {
		{ "key", '\0', 0, G_OPTION_ARG_STRING, &key,
		  "The public key, e.g. idle.timer_powerdown", NULL },
		{ "value", '\0', 0, G_OPTION_ARG_INT, &value,
		  "The value of the key, e.g. 1000", NULL },
		{ NULL}
	};

	context = g_option_context_new ("lsohm");
	g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
	g_option_context_parse (context, &argc, &argv, NULL);

	/* do nothing */
	if (key == NULL)
		return 0;

	g_type_init ();
	ctx = libohm_new ();
	error = NULL;
	ret = libohm_connect (ctx, &error);
	if (ret == FALSE) {
		g_warning ("cannot connect to ohmd: %s", error->message);
		g_error_free (error);
		goto unref;
	}

	/* returns list of all the LibOhmKeyValue on the system */
	error = NULL;
	ret = libohm_keystore_set_key (ctx, key, value, &error);
	if (ret == FALSE) {
		g_warning ("cannot set key: %s", error->message);
		g_error_free (error);
		goto unref;
	}

unref:
	g_object_unref (ctx);
	return 0;
}
