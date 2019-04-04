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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <glib.h>
#ifdef USE_GI18N
#include <glib/gi18n.h>
#endif
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <simple-trace/simple-trace.h>

#include "ohm-debug.h"
#include "ohm-common.h"
#include "ohm-manager.h"
#include "ohm-dbus-manager.h"
#include "ohm-dbus-internal.h"
#include "ohm/ohm-plugin-log.h"

#if _POSIX_MEMLOCK > 0
#  include <errno.h>
#  include <string.h>
#  include <sys/mman.h>

#  define MCL_DATA (MCL_FUTURE << 8)                    /* kludgish at best */
#endif

#if HAVE_MLOCKNICE_H
#  include <mlocknice.h>
#endif



#define MAX_TRACE_FLAGS 64

static GMainLoop *loop;
static int        verbosity;
static int        memlock = -1;
static char      *trace_flags[MAX_TRACE_FLAGS];
static int        num_flags = 0;

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


void
sighandler(int signum)
{
	if (signum == SIGINT) {
		g_debug ("Caught SIGINT, exiting");
		g_main_loop_quit (loop);
	}
}


gboolean
parse_verbosity(const gchar *option_name, const gchar *value,
		gpointer data, GError **error)
{
#define CHECK_ARG(s) (!strncmp(arg, (s), length))

	const char *arg, *next;
	int         length;

	/*
	 * handle -v -v -v type of setting
	 */

	if (value == NULL) {
		verbosity <<= 1;
		verbosity  |= 1;
		return TRUE;
	}


	/*
	 * handle --verbose=info,error type of settings
	 */
  
	for (arg = value; arg && *arg; arg = next) {
		next = strchr(arg, ',');

		if (next != NULL) {
			length = (int)(next - arg);
			next++;
		}
		else
			length = strlen(arg);
    
		if (CHECK_ARG("debug"))
		  	verbosity |= OHM_LOG_LEVEL_MASK(OHM_LOG_DEBUG);
		else if (CHECK_ARG("info"))
			verbosity |= OHM_LOG_LEVEL_MASK(OHM_LOG_INFO);
		else if (CHECK_ARG("warning"))
			verbosity |= OHM_LOG_LEVEL_MASK(OHM_LOG_WARNING);
		else if (CHECK_ARG("error"))
			verbosity |= OHM_LOG_LEVEL_MASK(OHM_LOG_ERROR);
		else if (CHECK_ARG("all") || CHECK_ARG("full"))
			verbosity = OHM_LOG_ALL;
		else if (CHECK_ARG("none"))
			verbosity = OHM_LOG_NONE;
		else {
			*error = g_error_new(G_OPTION_ERROR,
					     G_OPTION_ERROR_FAILED,
					     "invalid verbosity \"%*.*s\"",
					     length, length, arg);
			return FALSE;
		}
	}
  
	return TRUE;
}


#if _POSIX_MEMLOCK > 0
static gboolean
parse_memlock(const gchar *option_name, const gchar *value,
	      gpointer data, GError **error)
{
	if (value == NULL)
		memlock = MCL_CURRENT;
	else if (!strcmp(value, "none"))
		memlock = 0;
	else if (!strcmp(value, "current"))
		memlock = MCL_CURRENT;
	else if (!strcmp(value, "future"))
		memlock = MCL_FUTURE;
	else if (!strcmp(value, "all"))
		memlock = MCL_CURRENT | MCL_FUTURE;
	else if (!strcmp(value, "data"))
	  	memlock = MCL_DATA;
	else if (!strcmp(value, "data,future") ||
		 !strcmp(value, "future,data"))
		memlock = MCL_FUTURE | MCL_DATA;
	else if (!strcmp(value, "data,current") ||
		 !strcmp(value, "current,data"))
		memlock = MCL_CURRENT | MCL_DATA;
	else {
		*error = g_error_new(G_OPTION_ERROR,
				     G_OPTION_ERROR_FAILED,
				     "invalid mlock flag \"%s\"", value);
		memlock = 0;
		return FALSE;
	}
  
	return TRUE;
}
#endif


static void
lock_memory(OhmManager *mgr)
{
	int     lock, data;
	gchar  *what;
	GError *error = NULL;
	
	if (memlock < 0) {
		what = ohm_manager_get_string_option(mgr, "options", "mlock");
		if (!what)
			return;
		
		if (!parse_memlock("mlock", what, NULL, &error)) {
			OHM_ERROR("ohmd: invalid mlock option \"%s\".", what);
			g_error_free(error);
			return;
		}
	}
	
	if (!memlock)
		return;
	
	lock = memlock & ~MCL_DATA;
	data = memlock &  MCL_DATA;
	
	if (lock) {
		if (mlockall(lock) != 0)
			OHM_ERROR("ohmd: failed to lock address space (%s).",
				  strerror(errno));
		else
			OHM_INFO("ohmd: address space successfully locked.");
	}
	
	if (data) {
#if HAVE_MLOCKNICE_H
		if (mln_lock_data() != 0)
			OHM_ERROR("ohmd: failed to lock data segments (%s).",
				  strerror(errno));
		else
			OHM_INFO("ohmd: data segments successfully locked.");
#else
		OHM_WARNING("ohmd: compiled without mlocknice support.");
#endif
	}
}





static gboolean
parse_trace(const gchar *option_name, const gchar *value,
			    gpointer data, GError **error)
{
  	if (num_flags < MAX_TRACE_FLAGS)
		if (value && *value)
			trace_flags[num_flags++] = strdup(value);
	
	return TRUE;
}


static void
activate_trace(void)
{
	int  i;
	char help[32*1024];

	if (!num_flags)
		return;

	for (i = 0; i < num_flags; i++) {
		if (!strcmp(trace_flags[i], "help")) {
			printf("The possible plugin trace flags are:\n");
			trace_show(TRACE_DEFAULT_NAME, help, sizeof(help),
				   "  %-25.25F %-30.30d [%-3.3s]");
			exit(0);
		}
		else {
			trace_configure(trace_flags[i]);
			free(trace_flags[i]);
		}
	}

	trace_context_enable(TRACE_DEFAULT_CONTEXT);
}


int
ohm_verbosity(void)
{
	return verbosity;
}


#define MAX_ARGS 32
static char *saved_argv[MAX_ARGS + 1];

static void
save_args(int argc, char **argv)
{
	char exe[64];
	int  i;

	if (argc > MAX_ARGS)
	  argc = MAX_ARGS;

	sprintf(exe, "/proc/%u/exe", getpid());
	saved_argv[0] = strdup(argv[0]);

	for (i = 1; i < argc; i++)
		saved_argv[i] = strdup(argv[i]);
	saved_argv[i] = NULL;
}


void
ohm_restart(int delay)
{
  	int fd, max;

#ifdef _SC_OPEN_MAX
	max = (int)sysconf(_SC_OPEN_MAX);
#else
	max = 4096;
#endif

	OHM_INFO("ohmd: re-execing after %d seconds", delay);
	sleep(delay);
	
	for (fd = 3; fd < max; fd++)
		close(fd);

	sleep(1);
	execv(saved_argv[0], saved_argv);
}




/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	DBusGConnection *connection;
	gboolean no_daemon = FALSE;
	gboolean timed_exit = FALSE;
	gboolean g_fatal_warnings = FALSE;
	gboolean g_fatal_critical = FALSE;
	OhmManager *manager = NULL;
	GError *error = NULL;
	GOptionContext *context;
	
	const GOptionEntry entries[] = {
		{ "no-daemon", '\0', 0, G_OPTION_ARG_NONE, &no_daemon,
		  "Do not daemonize", NULL },
		{ "verbose", 'v', G_OPTION_FLAG_OPTIONAL_ARG,
		  G_OPTION_ARG_CALLBACK, parse_verbosity,
		  "Control level of debugging/logged information"
		  " (debug,info,warning,error)", NULL },
		{ "timed-exit", '\0', 0, G_OPTION_ARG_NONE, &timed_exit,
		  "Exit after a small delay (for debugging)", NULL },
		{ "g-fatal-warnings", 0, 0, G_OPTION_ARG_NONE, &g_fatal_warnings,
		  "Make all warnings fatal", NULL },
		{ "g-fatal-critical", 0, 0, G_OPTION_ARG_NONE, &g_fatal_critical,
		  "Make all critical warnings fatal", NULL },
#if _POSIX_MEMLOCK > 0
		{ "mlock", 'l', G_OPTION_FLAG_OPTIONAL_ARG,
		  G_OPTION_ARG_CALLBACK, parse_memlock,
		  "Lock process memory (none, current, future, all)", NULL },
#endif
		{ "trace", 't', G_OPTION_FLAG_OPTIONAL_ARG,
		  G_OPTION_ARG_CALLBACK, parse_trace,
		  "Set plugin trace flags", NULL },
		{ NULL}
	};

	setenv("TZ", "foo", 0);  /* my lips are sealed about this... */

	save_args(argc, argv);

	context = g_option_context_new (OHM_NAME);
#ifdef USE_GI18N
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
#else
	g_option_context_add_main_entries (context, entries, NULL);
#endif
	
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_critical("Failed to parse the command line.");
		fprintf(stderr, "Failed to parse the command line.\n");
		exit(1);
	}

	g_option_context_free (context);

	if (g_fatal_warnings || g_fatal_critical)
	{
		GLogLevelFlags fatal_mask;
	
		g_debug("setting fatal warnings");
		fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
		fatal_mask |= (g_fatal_warnings?G_LOG_LEVEL_WARNING:0) | G_LOG_LEVEL_CRITICAL;
		g_log_set_always_fatal (fatal_mask);
	}

#if (GLIB_MAJOR_VERSION <= 2) && (GLIB_MINOR_VERSION < 36)
	g_type_init ();
#endif

#ifdef HAVE_GTHREAD
	if (!g_thread_supported ())
		g_thread_init (NULL);
	dbus_g_thread_init ();
#endif

	/* we need to daemonize before we get a system connection */
	if (no_daemon == FALSE) {
		if (daemon (0, 0)) {
			g_error ("Could not daemonize.");
		}
	}
	ohm_debug_init (verbosity & OHM_LOG_LEVEL_MASK(OHM_LOG_DEBUG));
	ohm_log_init (verbosity);

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

	activate_trace();

	ohm_debug ("Idle");
	loop = g_main_loop_new (NULL, FALSE);

	lock_memory(manager);

	/* Only timeout and close the mainloop if we have specified it
	 * on the command line */
	if (timed_exit == FALSE) {
		g_main_loop_run (loop);
	}

	
	g_object_unref (manager);

	ohm_dbus_exit();

	dbus_g_connection_unref (connection);

	/* free memory used by dbus  */
	dbus_shutdown();
	
	return 0;
}
