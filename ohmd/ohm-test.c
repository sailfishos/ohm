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
#  include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "ohm-debug.h"
#include "ohm-common.h"
#include "ohm-confobj.h"

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	OhmConfObj *confobj = NULL;

	g_type_init ();

	ohm_debug_init (TRUE);

	ohm_debug ("Creating confobj");
	confobj = ohm_confobj_new ();

	g_object_unref (confobj);

	return 0;
}
