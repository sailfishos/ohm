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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <gmodule.h>
#include <glib.h>
#include <stdlib.h>

#include <ohm-plugin.h>

static gboolean
plugin_poll_startup (gpointer data)
{
	OhmPlugin *plugin = (OhmPlugin *) data;
	gboolean ret;
	const gchar *xauth;

#ifdef OHM_SINGLE_USER_DEVICE
	xauth = OHM_DEVICE_XAUTH_DIR "/.Xauthority";
	g_debug ("xorg: testing %s", xauth);
	ret = g_file_test (xauth, G_FILE_TEST_EXISTS);

	/* be nice to developers... */
	if (ret == FALSE) {
		const char *home = getenv("HOME");
		if (home != NULL) {
			xauth = g_strdup_printf ("%s/.Xauthority", home);
			g_debug ("xorg: testing %s", xauth);
			ret = g_file_test (xauth, G_FILE_TEST_EXISTS);
		}
	}
	if (ret == FALSE) {
		g_debug ("xorg: no .Xauthority found");
		return TRUE;
	}
#else
#error ConsoleKit support not yet implemented
#endif
	/* woot! X is alive */
	g_debug ("Got X!");
	ohm_plugin_conf_set_key (plugin, "xorg.has_xauthority", 1);
	setenv ("XAUTHORITY", xauth, 1);
	setenv ("DISPLAY", ":0", 1);
	/* we don't need to poll anymore */
	return FALSE;
}

/**
 * plugin_initialize:
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_initialize (OhmPlugin *plugin)
{
	gboolean ret;

	/* we can assume xorg is on */
	ohm_plugin_conf_set_key (plugin, "xorg.has_xauthority", 0);

	/* naive try... we might be running under X */
	ret = plugin_poll_startup (plugin);
	if (ret == FALSE) {
		/* we failed to connect to X - maybe X is not alive yet? */
		g_debug ("Could not connect to X, polling until we can");
		g_timeout_add (2000, plugin_poll_startup, plugin);
	}
}

OHM_PLUGIN_DESCRIPTION (
	"OHM xorg finder",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	OHM_LICENSE_LGPL,		/* license */
	plugin_initialize,		/* initialize */
	NULL,				/* destroy */
	NULL				/* notify */
);

OHM_PLUGIN_PROVIDES ("xorg.has_xauthority");
