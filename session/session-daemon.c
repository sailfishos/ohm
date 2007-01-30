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

#include <string.h>
#include <unistd.h>
#include <glib.h>

#include <libohm.h>

/* best effort */
static void
ohm_set_value_be (LibOhm *ctx, const char *key, int value)
{
	GError *error;
	gboolean ret;

	error = NULL;
	ret = libohm_keystore_set_key (ctx, key, value, &error);
	if (ret == FALSE) {
		g_debug ("Setting key '%s' failed: %s", key, error->message);
		g_error_free (error);
	}
}

/* best effort */
static int
ohm_get_value_be (LibOhm *ctx, const char *key)
{
	GError *error;
	gboolean ret;
	gint value;

	error = NULL;
	ret = libohm_keystore_get_key (ctx, key, &value, &error);
	if (ret == FALSE) {
		g_debug ("Getting key '%s' failed: %s", key, error->message);
		g_error_free (error);
		value = -1;
	}
	return value;
}

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	LibOhm *ctx;
	gboolean ret;
	gint value;
	gchar *version = NULL;
	GError *error;

	g_type_init ();

	g_debug ("Creating ctx");
	ctx = libohm_new ();
	error = NULL;
	ret = libohm_connect (ctx, &error);
	if (ret == FALSE) {
		g_error ("failed to connect: %s", error->message);
		g_error_free (error);
	}

	ret = libohm_server_get_version (ctx, &version, NULL);
	if (ret && strcmp (version, "0.0.1") != 0) {
		g_warning ("not compiled for this version of the server, may have errors");
	}
	g_free (version);

	ohm_set_value_be (ctx, "timeremaining.calculate_hysteresis", 0);
	ohm_set_value_be (ctx, "powerstatus.percentage_low", 20);
	ohm_set_value_be (ctx, "powerstatus.percentage_critical", 10);
	ohm_set_value_be (ctx, "dpms.method", 3);
	ohm_set_value_be (ctx, "backlight.value_idle", 40);
	ohm_set_value_be (ctx, "backlight.value_battery", 70);
	ohm_set_value_be (ctx, "backlight.value_ac", 100);
	ohm_set_value_be (ctx, "backlight.time_off", 400);
	ohm_set_value_be (ctx, "backlight.time_idle", 200);

	value = ohm_get_value_be (ctx, "backlight.time_idle");
	g_debug ("value=%i", value);

	g_object_unref (ctx);

	return 0;
}
