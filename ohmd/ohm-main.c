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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "ohm-debug.h"
#include "ohm-common.h"
#include "ohm-manager.h"
#include "ohm-dbus-manager.h"
#include "ohm-dbus.h"

static GMainLoop *loop;

/**
 * ohm_object_register:
 * @connection: What we want to register to
 * @object: The GObject we want to register
 *
 * Register org.freedesktop.ohm on the session bus.
 * This function MUST be called before DBUS service will work.
 *
 * Return value: success
 **/
static gboolean
ohm_object_register (DBusGConnection *connection,
		     GObject	     *object)
{
	DBusGProxy *bus_proxy = NULL;
	GError *error = NULL;
	guint request_name_result;
	gboolean ret;

	bus_proxy = dbus_g_proxy_new_for_name (connection,
					       DBUS_SERVICE_DBUS,
					       DBUS_PATH_DBUS,
					       DBUS_INTERFACE_DBUS);

	ret = dbus_g_proxy_call (bus_proxy, "RequestName", &error,
				 G_TYPE_STRING, OHM_DBUS_SERVICE,
				 G_TYPE_UINT, 0,
				 G_TYPE_INVALID,
				 G_TYPE_UINT, &request_name_result,
				 G_TYPE_INVALID);
	if (error) {
		ohm_debug ("ERROR: %s", error->message);
		g_error_free (error);
	}
	if (ret == FALSE) {
		/* abort as the DBUS method failed */
		g_warning ("RequestName failed!");
		return FALSE;
	}

	/* free the bus_proxy */
	g_object_unref (G_OBJECT (bus_proxy));

	/* already running */
 	if (request_name_result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_warning ("Already running!");
		return FALSE;
	}

	dbus_g_object_type_install_info (OHM_TYPE_MANAGER, &dbus_glib_ohm_manager_object_info);
	dbus_g_connection_register_g_object (connection, OHM_DBUS_PATH_MANAGER, object);

	return TRUE;
}


void sighandler(int signum)
{
	if (signum == SIGINT) {
		g_debug ("Caught SIGINT, exiting");
		g_main_loop_quit (loop);
	}
}

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	DBusGConnection *connection;
	gboolean verbose = FALSE;
	gboolean no_daemon = FALSE;
	gboolean timed_exit = FALSE;
	gboolean g_fatal_warnings = FALSE;
	gboolean g_fatal_critical = FALSE;
	OhmManager *manager = NULL;
	GError *error = NULL;
	GOptionContext *context;

	const GOptionEntry entries[] = {
		{ "no-daemon", '\0', 0, G_OPTION_ARG_NONE, &no_daemon,
		  N_("Do not daemonize"), NULL },
		{ "verbose", '\0', 0, G_OPTION_ARG_NONE, &verbose,
		  N_("Show extra debugging information"), NULL },
		{ "timed-exit", '\0', 0, G_OPTION_ARG_NONE, &timed_exit,
		  N_("Exit after a small delay (for debugging)"), NULL },
		{ "g-fatal-warnings", 0, 0, G_OPTION_ARG_NONE, &g_fatal_warnings,
		  N_("Make all warnings fatal"), NULL },
		{ "g-fatal-critical", 0, 0, G_OPTION_ARG_NONE, &g_fatal_critical,
		  N_("Make all critical warnings fatal"), NULL },
		{ NULL}
	};

	context = g_option_context_new (OHM_NAME);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free (context);

	if (g_fatal_warnings || g_fatal_critical)
	{
		GLogLevelFlags fatal_mask;
	
		g_debug("setting fatal warnings");
		fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
		fatal_mask |= (g_fatal_warnings?G_LOG_LEVEL_WARNING:0) | G_LOG_LEVEL_CRITICAL;
		g_log_set_always_fatal (fatal_mask);
	}

	g_type_init ();
	if (!g_thread_supported ())
		g_thread_init (NULL);
	dbus_g_thread_init ();

	/* we need to daemonize before we get a system connection */
	if (no_daemon == FALSE) {
		if (daemon (0, 0)) {
			g_error ("Could not daemonize.");
		}
	}
	ohm_debug_init (verbose);

	/* check dbus connections, exit if not valid */
	connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (error) {
		g_warning ("%s", error->message);
		g_error_free (error);
		g_error ("This program cannot start until you start "
			 "the dbus system service.");
	}

	ohm_debug("Initializing DBUS helper");
	if (!ohm_dbus_init(connection))
	  g_error("%s failed to start.", OHM_NAME);
	

	ohm_debug ("Creating manager");
	manager = ohm_manager_new ();
	if (!ohm_object_register (connection, G_OBJECT (manager))) {
		g_error ("%s failed to start.", OHM_NAME);
		return 0;
	}

	signal (SIGINT, sighandler);

	ohm_debug ("Idle");
	loop = g_main_loop_new (NULL, FALSE);

	/* Only timeout and close the mainloop if we have specified it
	 * on the command line */
	if (timed_exit == FALSE) {
		g_main_loop_run (loop);
	}

	
	g_object_unref (manager);

	ohm_dbus_exit();

	dbus_g_connection_unref (connection);
	/*free memory used by dbus*/

	dbus_shutdown();

	return 0;
}
