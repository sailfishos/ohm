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

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "ohm-debug.h"

static gboolean do_debug = FALSE;

/**
 * ohm_debug_real:
 **/
void
ohm_debug_real (const gchar *func,
		const gchar *file,
		const int    line,
		const gchar *format, ...)
{
	va_list args;
	gchar va_args_buffer [1025];
	gchar   *str_time;
	time_t  the_time;


	if (do_debug == FALSE) {
		return;
	}

	va_start (args, format);
	g_vsnprintf (va_args_buffer, 1024, format, args);
	va_end (args);

	time (&the_time);
	str_time = g_new0 (gchar, 255);
	strftime (str_time, 254, "%H:%M:%S", localtime (&the_time));

	fprintf (stderr, "[%s] %s:%d (%s):\t %s\n",
		 func, file, line, str_time, va_args_buffer);
	g_free (str_time);
}

/**
 * ohm_debug_init:
 * @debug: If we should print out verbose logging
 **/
void
ohm_debug_init (gboolean debug)
{
	do_debug = debug;
	ohm_debug ("Verbose debugging %s", (do_debug) ? "enabled" : "disabled");
}
