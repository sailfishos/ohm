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

#ifndef __OHMCOMMON_H
#define __OHMCOMMON_H

#include <glib.h>

G_BEGIN_DECLS

/* common descriptions of this program */
#define OHM_NAME 			_("Open Hardware Manager")
#define OHM_DESCRIPTION 		_("Hardware Management Daemon")

#define	OHM_DBUS_SERVICE		"org.freedesktop.ohm"
#define	OHM_DBUS_INTERFACE_KEYSTORE	"org.freedesktop.ohm.Keystore"
#define	OHM_DBUS_INTERFACE_MANAGER	"org.freedesktop.ohm.Manager"
#define	OHM_DBUS_PATH_KEYSTORE		"/org/freedesktop/ohm/Keystore"
#define	OHM_DBUS_PATH_MANAGER		"/org/freedesktop/ohm/Manager"

G_END_DECLS

#endif	/* __OHMCOMMON_H */
