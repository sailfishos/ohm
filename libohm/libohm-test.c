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

#include "libohm.h"

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	LibOhm *ctx;
	gboolean ret;
	gint value;

	g_type_init ();

	g_debug ("Creating ctx");
	ctx = libohm_new ();

	ret = libohm_keystore_set_key (ctx, "backlight.value_idle", 999, NULL);
	g_debug ("ret=%i", ret);

	ret = libohm_keystore_get_key (ctx, "backlight.value_idle", &value, NULL);
	g_debug ("ret=%i, value=%i", ret, value);

	g_object_unref (ctx);

	return 0;
}
