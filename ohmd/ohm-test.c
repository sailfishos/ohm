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
#include "ohm-conf.h"
#include "ohm-confobj.h"

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	OhmConf *conf = NULL;
	OhmConfObj *confobj = NULL;
	gboolean ret;
	GError *error;

	g_type_init ();

	ohm_debug_init (TRUE);

	ohm_debug ("Tesing confobj");
	confobj = ohm_confobj_new ();
	g_object_unref (confobj);

	ohm_debug ("Testing conf");
	conf = ohm_conf_new ();

	/* add a public key */
	error = NULL;
	ret = ohm_conf_add_key (conf, "backlight.time_off", 101, TRUE, &error);
	if (ret == FALSE) {
		g_error ("add: %s", error->message);
	}

	/* test the multi-user conf stuff */
	error = NULL;
	ret = ohm_conf_user_add (conf, "hughsie", &error);
	if (ret == FALSE) {
		g_error ("add: %s", error->message);
	}

	ohm_debug ("set 999 for root");
	error = NULL;
	ret = ohm_conf_set_key_internal (conf, "backlight.time_off", 999, TRUE, &error);
	if (ret == FALSE) {
		g_error ("set: %s", error->message);
	}

	gint value;
	error = NULL;
	ret = ohm_conf_get_key (conf, "backlight.time_off", &value, &error);
	if (ret == FALSE) {
		g_error ("get: %s", error->message);
	}
	ohm_debug ("got for root %i (should be 999)", value);

	ohm_debug ("switch to hughsie");
	error = NULL;
	ret = ohm_conf_user_switch (conf, "hughsie", &error);
	if (ret == FALSE) {
		ohm_debug ("switch: %s", error->message);
	}

	error = NULL;
	ret = ohm_conf_get_key (conf, "backlight.time_off", &value, &error);
	if (ret == FALSE) {
		g_error ("get: %s", error->message);
	}
	ohm_debug ("got for hughsie %i (should be 999)", value);

	ohm_debug ("set 101 for hughsie");
	error = NULL;
	ret = ohm_conf_set_key_internal (conf, "backlight.time_off", 101, TRUE, &error);
	if (ret == FALSE) {
		g_error ("set: %s", error->message);
	}

	error = NULL;
	ret = ohm_conf_get_key (conf, "backlight.time_off", &value, &error);
	if (ret == FALSE) {
		g_error ("get: %s", error->message);
	}
	ohm_debug ("got for hughsie %i (should be 101)", value);

	ohm_debug ("switch to root");
	error = NULL;
	ret = ohm_conf_user_switch (conf, "root", &error);
	if (ret == FALSE) {
		ohm_debug ("switch: %s", error->message);
	}

	error = NULL;
	ret = ohm_conf_get_key (conf, "backlight.time_off", &value, &error);
	if (ret == FALSE) {
		g_error ("get: %s", error->message);
	}
	ohm_debug ("got for root %i (should be 999)", value);

	ohm_debug ("remove hughsie");
	error = NULL;
	ret = ohm_conf_user_remove (conf, "hughsie", &error);
	if (ret == FALSE) {
		ohm_debug ("remove: %s", error->message);
	}

	ohm_conf_user_list (conf);

	g_object_unref (conf);

	return 0;
}
