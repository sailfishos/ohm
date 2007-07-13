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

/**
 * plugin_preload:
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static gboolean
plugin_preload (OhmPlugin *plugin)
{
	/* add in the required, suggested and prevented plugins */
	ohm_plugin_conf_provide (plugin, "xorg.has_xauthority");
	return TRUE;
}

static gboolean
plugin_poll_startup (gpointer data)
{
	OhmPlugin *plugin = (OhmPlugin *) data;
	gboolean ret;
	const gchar *xauth;

	/* we should search all of home */
	xauth = "/home/olpc/.Xauthority";
	ret = g_file_test (xauth, G_FILE_TEST_EXISTS);

	/* be nice to hughsie... */
	if (ret == FALSE) {
		xauth = "/home/hughsie/.Xauthority";
		ret = g_file_test (xauth, G_FILE_TEST_EXISTS);
	}
	if (ret == FALSE) {
		/* not yet */
		return TRUE;
	}

	/* woot! X is alive */
	g_debug ("Got X!");
	ohm_plugin_conf_set_key (plugin, "xorg.has_xauthority", 1);
	setenv ("XAUTHORITY", xauth, 1);
	setenv ("DISPLAY", ":0", 1);
	/* we don't need to poll anymore */
	return FALSE;
}

/**
 * plugin_coldplug:
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_coldplug (OhmPlugin *plugin)
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

static OhmPluginInfo plugin_info = {
	"OHM xorg finder",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	NULL,				/* unload */
	plugin_coldplug,		/* coldplug */
	NULL,				/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);
