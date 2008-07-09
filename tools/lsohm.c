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
	GSList *list = NULL;
	GSList *l;
	GOptionContext *context;
	gboolean ret;
	gboolean use_public = FALSE;
	gboolean use_private = FALSE;
	gchar *spaces;
	guint max = 0;
	guint len;
	LibOhm *ctx;
	LibOhmKeyValue *keyvalue;

	const GOptionEntry options[] = {
		{ "public", '\0', 0, G_OPTION_ARG_NONE, &use_public,
		  "Print public keys", NULL },
		{ "private", '\0', 0, G_OPTION_ARG_NONE, &use_private,
		  "Print private keys", NULL },
		{ NULL}
	};

	context = g_option_context_new ("lsohm");
	g_option_context_add_main_entries (context, options, NULL);
	g_option_context_parse (context, &argc, &argv, NULL);

	/* if neither specified, do both */
	if (use_public == FALSE && use_private == FALSE) {
		use_public = TRUE;
		use_private = TRUE;
	}

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
	ret = libohm_keystore_get_keys (ctx, &list, &error);
	if (ret == FALSE) {
		g_warning ("cannot get keys: %s", error->message);
		g_error_free (error);
		goto unref;
	}

	/* get the max length of the name part of the keys */
	for (l=list; l != NULL; l=l->next) {
		keyvalue = (LibOhmKeyValue *) l->data;
		len = strlen (keyvalue->name);
		if (len > max) {
			max = len;
		}
	}

	/* print the keys */
	for (l=list; l != NULL; l=l->next) {
		keyvalue = (LibOhmKeyValue *) l->data;
		spaces = g_strnfill (max - strlen (keyvalue->name), ' ');
		if (use_public == TRUE && keyvalue->public == TRUE) {
			g_print ("%s%s: %i\t(public)\n", keyvalue->name, spaces, keyvalue->value);
		} else if (use_private == TRUE && keyvalue->public == FALSE) {
			g_print ("%s%s: %i\t(private)\n", keyvalue->name, spaces, keyvalue->value);
		}
		g_free (spaces);
	}

	/* free the keys */
	libohm_keystore_free_keys (ctx, list);
unref:
	g_object_unref (ctx);
	return 0;
}
