/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libidletime.h"

static void     idletime_class_init (LibIdletimeClass *klass);
static void     idletime_init       (LibIdletime      *idletime);
static void     idletime_finalize   (GObject       *object);

#define LIBIDLETIME_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), LIBIDLETIME_TYPE, LibIdletimePrivate))

inline gint64
xsyncvalue_to_int64 (XSyncValue *value)
{
	return ((guint64) XSyncValueHigh32 (*value)) << 32
		| (guint64) XSyncValueLow32 (*value);
}

inline XSyncValue
int64_to_xsyncvalue (gint64 value)
{
	XSyncValue ret;
	XSyncIntsToValue (&ret, value, ((guint64)value) >> 32);
	return ret;
}

struct LibIdletimePrivate
{
	int			 sync_event;
	guint			 last_event;
	gboolean		 reset_set;
	XSyncCounter		 idle_counter;
	GPtrArray		*array;
	Display			*dpy;
};

typedef struct
{
	guint      id;
	XSyncValue timeout;
	XSyncAlarm xalarm;
} LibIdletimeAlarm;

enum {
	ALARM_EXPIRED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };
static gpointer idletime_object = NULL;

G_DEFINE_TYPE (LibIdletime, idletime, G_TYPE_OBJECT)

/**
 * idletime_xsync_alarm_set:
 *
 * Gets the time remaining for the current percentage
 *
 */
static gboolean
idletime_xsync_alarm_set (LibIdletime *idletime, LibIdletimeAlarm *alarm, gboolean positive)
{
	XSyncAlarmAttributes attr;
	XSyncValue delta;
	unsigned int flags;
	XSyncTestType test;

	if (idletime->priv->dpy == NULL) {
		return FALSE;
	}

	/* which way do we do the test? */
	if (positive == TRUE) {
		test = XSyncPositiveComparison;
	} else {
		test = XSyncNegativeComparison;
	}

	XSyncIntToValue (&delta, 0);

	attr.trigger.counter = idletime->priv->idle_counter;
	attr.trigger.value_type = XSyncAbsolute;
	attr.trigger.test_type = test;
	attr.trigger.wait_value = alarm->timeout;
	attr.delta = delta;

	flags = XSyncCACounter | XSyncCAValueType | XSyncCATestType | XSyncCAValue | XSyncCADelta;

	if (alarm->xalarm) {
		XSyncChangeAlarm (idletime->priv->dpy, alarm->xalarm, flags, &attr);
	} else {
		alarm->xalarm = XSyncCreateAlarm (idletime->priv->dpy, flags, &attr);
	}
	return TRUE;
}

/**
 * idletime_alarm_reset_all:
 */
void
idletime_alarm_reset_all (LibIdletime *idletime)
{
	guint i;
	LibIdletimeAlarm *alarm;

	/* reset all the alarms (except the reset alarm) to their timeouts */
	for (i=1; i<idletime->priv->array->len; i++) {
		alarm = g_ptr_array_index (idletime->priv->array, i);
		idletime_xsync_alarm_set (idletime, alarm, TRUE);
	}

	/* emit signal */
	g_signal_emit (idletime, signals [ALARM_EXPIRED], 0, 0);

	/* we need to be reset again on the next event */
	idletime->priv->reset_set = FALSE;
}

/**
 * idletime_timeout:
 */
static void
idletime_timeout (LibIdletime *idletime, LibIdletimeAlarm *alarm)
{
	/* emit signal */
	g_signal_emit (idletime, signals [ALARM_EXPIRED], 0, alarm->id);
}

/**
 * idletime_alarm_find_id:
 */
static LibIdletimeAlarm *
idletime_alarm_find_id (LibIdletime *idletime, guint id)
{
	guint i;
	LibIdletimeAlarm *alarm;
	for (i=0; i<idletime->priv->array->len; i++) {
		alarm = g_ptr_array_index (idletime->priv->array, i);
		if (alarm->id == id) {
			return alarm;
		}
	}
	return NULL;
}

/**
 * idletime_x_set_reset:
 */
static void
idletime_x_set_reset (LibIdletime *idletime, XSyncAlarmNotifyEvent *alarm_event)
{
	LibIdletimeAlarm *alarm;

	alarm = idletime_alarm_find_id (idletime, 0);

	if (idletime->priv->reset_set == FALSE) {
		/* don't match on the current value because
		 * XSyncNegativeComparison means less or equal. */
		alarm->timeout = int64_to_xsyncvalue (xsyncvalue_to_int64 (&alarm_event->counter_value) - 1LL);

		/* set the reset alarm to fire the next time
		 * idletime->priv->idle_counter < the current counter value */
		idletime_xsync_alarm_set (idletime, alarm, FALSE);

		/* don't try to set this again */
		idletime->priv->reset_set = TRUE;
	}
}

/**
 * idletime_alarm_find_event:
 */
static LibIdletimeAlarm *
idletime_alarm_find_event (LibIdletime *idletime, XSyncAlarmNotifyEvent *alarm_event)
{
	guint i;
	LibIdletimeAlarm *alarm;
	for (i=0; i<idletime->priv->array->len; i++) {
		alarm = g_ptr_array_index (idletime->priv->array, i);
		if (alarm_event->alarm == alarm->xalarm) {
			return alarm;
		}
	}
	return NULL;
}

/**
 * idletime_x_event_filter:
 */
static GdkFilterReturn
idletime_x_event_filter (GdkXEvent *gdkxevent, GdkEvent *event, gpointer data)
{
	LibIdletimeAlarm *alarm;
	XEvent *xevent = (XEvent *) gdkxevent;
	LibIdletime *idletime = (LibIdletime *) data;
	XSyncAlarmNotifyEvent *alarm_event;

	/* no point continuing */
	if (xevent->type != idletime->priv->sync_event + XSyncAlarmNotify)
		return GDK_FILTER_CONTINUE;

	alarm_event = (XSyncAlarmNotifyEvent *) xevent;

	alarm = idletime_alarm_find_event (idletime, alarm_event);

	/* did we match one of our alarms? */
	if (alarm != NULL) {
		/* save the last state we triggered */
		idletime->priv->last_event = alarm->id;

		/* do the signal */
		if (alarm->id != 0) {
			idletime_timeout (idletime, alarm);

			/* we need the first alarm to go off to set the reset alarm */
			idletime_x_set_reset (idletime, alarm_event);
			return GDK_FILTER_CONTINUE;
		} else {
			/* do the reset callback */
			idletime_alarm_reset_all (idletime);
		}
	}

	return GDK_FILTER_CONTINUE;
}

/**
 * idletime_get_idletime->priv->last_event:
 */
guint
idletime_alarm_get (LibIdletime *idletime)
{
	return idletime->priv->last_event;
}

/**
 * idletime_alarm_set:
 */
gboolean
idletime_alarm_set (LibIdletime *idletime, guint id, gint64 timeout)
{
	LibIdletimeAlarm *alarm;
	if (id == 0) {
		/* id cannot be zero */
		return FALSE;
	}
	if (timeout == 0) {
		/* timeout cannot be zero */
		return FALSE;
	}

	/* see if we already created an alarm with this ID */
	alarm = idletime_alarm_find_id (idletime, id);
	if (alarm == NULL) {
		/* create a new alarm */
		alarm = g_new0 (LibIdletimeAlarm, 1);

		/* set the id - this is just something like userdata */
		alarm->id = id;

		/* add to array */
		g_ptr_array_add (idletime->priv->array, alarm);
	}

	/* set the timeout */
	alarm->timeout = int64_to_xsyncvalue(timeout);

	/* set, and start the timer */
	idletime_xsync_alarm_set (idletime, alarm, TRUE);
	return TRUE;
}

/**
 * idletime_alarm_free:
 */
static gboolean
idletime_alarm_free (LibIdletime *idletime, LibIdletimeAlarm *alarm)
{
	if (idletime->priv->dpy == NULL) {
		return FALSE;
	}
	XSyncDestroyAlarm (idletime->priv->dpy, alarm->xalarm);
	g_free (alarm);
	g_ptr_array_remove (idletime->priv->array, alarm);
	return TRUE;
}

/**
 * idletime_alarm_free:
 */
gboolean
idletime_alarm_remove (LibIdletime *idletime, guint id)
{
	LibIdletimeAlarm *alarm;
	alarm = idletime_alarm_find_id (idletime, id);
	if (alarm == NULL) {
		return FALSE;
	}
	idletime_alarm_free (idletime, alarm);
	return TRUE;
}

/**
 * idletime_get_current_idle:
 * @idletime: An #LibIdletime instantiation
 * 
 * Returns the current number of milliseconds idle
 */
gint64
idletime_get_current_idle (LibIdletime *idletime)
{
	XSyncValue value;
	if (XSyncQueryCounter (idletime->priv->dpy, idletime->priv->idle_counter, &value))
		return xsyncvalue_to_int64 (&value);
	else
		return 0LL;
}

/**
 * idletime_class_init:
 * @klass: This class instance
 **/
static void
idletime_class_init (LibIdletimeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = idletime_finalize;
	g_type_class_add_private (klass, sizeof (LibIdletimePrivate));

	signals [ALARM_EXPIRED] =
		g_signal_new ("alarm-expired",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (LibIdletimeClass, alarm_expired),
			      NULL,
			      NULL,
			      g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE,
			      1, G_TYPE_UINT);
}

/**
 * idletime_connect_x:
 **/
static gboolean
idletime_connect_x (LibIdletime *idletime)
{
	int i;
	int sync_error;
	int ncounters;
	XSyncSystemCounter *counters;
	gboolean ret;

	ret = gdk_init_check (NULL, NULL);
	if (ret == FALSE) {
		/* we failed */
		return FALSE;
	}

	/* are we started in X? */
	idletime->priv->dpy = GDK_DISPLAY ();

#if 0
	if (idletime->priv->dpy == NULL) {
		/* try using root display */
		gdk_error_trap_push ();
		idletime->priv->dpy = XOpenDisplay (NULL);
		if (gdk_error_trap_pop ()) {
			g_warning ("No NULL connection");
		}
	}
	if (idletime->priv->dpy == NULL) {
		/* try using root display :0 */
		gdk_error_trap_push ();
		idletime->priv->dpy = XOpenDisplay (":0");
		if (gdk_error_trap_pop ()) {
			g_warning ("No :0 connection");
		}
	}
	if (idletime->priv->dpy == NULL) {
		/* try using root display :0.0 */
		gdk_error_trap_push ();
		idletime->priv->dpy = XOpenDisplay (":0.0");
		if (gdk_error_trap_pop ()) {
			g_warning ("No :0.0 connection");
		}
	}
#endif

	/* bugger */
	if (idletime->priv->dpy == NULL) {
		g_debug ("no X connection!");
		return FALSE;
	}

	g_debug ("Using X dpy : %p", idletime->priv->dpy);

	/* get the sync event */
	if (!XSyncQueryExtension (idletime->priv->dpy, &idletime->priv->sync_event, &sync_error)) {
		g_warning ("No Sync extension.");
		return FALSE;
	}

	/* gtk_init should do XSyncInitialize for us */
	counters = XSyncListSystemCounters (idletime->priv->dpy, &ncounters);
	for (i = 0; i < ncounters && !idletime->priv->idle_counter; i++) {
		if (!strcmp(counters[i].name, "IDLETIME"))
			idletime->priv->idle_counter = counters[i].counter;
	}
//	XSyncFreeSystemCounterList (counters);

	/* arh. we don't have IDLETIME support */
	if (!idletime->priv->idle_counter) {
		g_warning ("No idle counter.");
		return FALSE;
	}

	idletime->priv->reset_set = FALSE;

	/* only match on XSyncAlarmNotifyMask */
	gdk_error_trap_push ();
	XSelectInput (idletime->priv->dpy, GDK_ROOT_WINDOW (), XSyncAlarmNotifyMask);
	if (gdk_error_trap_pop ()) {
		g_warning ("XSelectInput failed");
	}

	/* this is where we want events */
	gdk_window_add_filter (NULL, idletime_x_event_filter, idletime);
	return TRUE;
}

static gboolean
idletime_poll_startup (gpointer data)
{
	LibIdletime *idletime = (LibIdletime *) data;
	gboolean ret;
	g_debug ("Try to connect to X");
	ret = idletime_connect_x (idletime);
	if (ret == TRUE) {
		g_debug ("Got X!");
		/* we don't need to poll anymore */
		return FALSE;
	}
	return TRUE;
}

/**
 * idletime_init:
 *
 * @idletime: This class instance
 **/
static void
idletime_init (LibIdletime *idletime)
{
	LibIdletimeAlarm *alarm;
	gboolean ret;

	idletime->priv = LIBIDLETIME_GET_PRIVATE (idletime);

	idletime->priv->array = g_ptr_array_new ();

	idletime->priv->idle_counter = None;
	idletime->priv->last_event = 0;
	idletime->priv->sync_event = 0;
	idletime->priv->dpy = NULL;

	ret = idletime_connect_x (idletime);
	if (ret == FALSE) {
		/* we failed to connect to X - maybe X is not alive yet? */
		g_debug ("Could not connect to X, polling until we can");
		g_timeout_add (2000, idletime_poll_startup, idletime);
	}

	/* create a reset alarm */
	alarm = g_new0 (LibIdletimeAlarm, 1);
	alarm->id = 0;
	g_ptr_array_add (idletime->priv->array, alarm);
}

/**
 * idletime_finalize:
 * @object: This class instance
 **/
static void
idletime_finalize (GObject *object)
{
	guint i;
	LibIdletime *idletime;
	LibIdletimeAlarm *alarm;

	g_return_if_fail (object != NULL);
	g_return_if_fail (LIBIDLETIME_IS (object));

	idletime = LIBIDLETIME (object);
	idletime->priv = LIBIDLETIME_GET_PRIVATE (idletime);

	for (i=0; i<idletime->priv->array->len; i++) {
		alarm = g_ptr_array_index (idletime->priv->array, i);
		idletime_alarm_free (idletime, alarm);
	}
	g_ptr_array_free (idletime->priv->array, TRUE);

	G_OBJECT_CLASS (idletime_parent_class)->finalize (object);
}

/**
 * idletime_new:
 * Return value: new LibIdletime instance.
 **/
LibIdletime *
idletime_new (void)
{
	if (idletime_object != NULL) {
		g_object_ref (idletime_object);
	} else {
		idletime_object = g_object_new (LIBIDLETIME_TYPE, NULL);
		g_object_add_weak_pointer (idletime_object, &idletime_object);
	}
	return LIBIDLETIME (idletime_object);
}

