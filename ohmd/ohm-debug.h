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

#ifndef _OHM_DEBUG_H
#define _OHM_DEBUG_H

#include <stdarg.h>
#include <glib.h>

G_BEGIN_DECLS

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define ohm_debug(...) ohm_debug_real (__func__, __FILE__, __LINE__, __VA_ARGS__)
#elif defined(__GNUC__) && __GNUC__ >= 3
#define ohm_debug(...) ohm_debug_real (__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define ohm_debug
#endif

void		ohm_debug_init			(gboolean	 debug);
void		ohm_debug_shutdown		(void);
void		ohm_debug_real			(const gchar	*func,
						 const gchar	*file,
						 int		 line,
						 const gchar	*format, ...);
void		ohm_warning_real		(const gchar	*func,
						 const gchar	*file,
						 int		 line,
						 const gchar	*format, ...);

G_END_DECLS

#endif /* _OHM_DEBUG_H */
