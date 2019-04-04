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

#ifndef __OHM_PLUGIN_INTERNAL_H
#define __OHM_PLUGIN_INTERNAL_H

#include <glib-object.h>
#include <ohm/ohm-plugin.h>
#include <ohm/ohm-plugin-dbus.h>


G_BEGIN_DECLS

#define OHM_TYPE_PLUGIN		(ohm_plugin_get_type ())
#define OHM_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_PLUGIN, OhmPlugin))
#define OHM_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_PLUGIN, OhmPluginClass))
#define OHM_IS_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_PLUGIN))
#define OHM_IS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_PLUGIN))
#define OHM_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_PLUGIN, OhmPluginClass))

typedef struct _OhmPluginPrivate OhmPluginPrivate;
typedef struct _OhmPluginClass OhmPluginClass;

struct _OhmPlugin
{
	GObject		 parent;
	OhmPluginDesc   *desc;
	const OhmPluginKeyIdMap *interested;
	const char **provides;
	const char **requires;
	const char **suggests;
	const char **prevents;

        ohm_dbus_method_t   *dbus_methods;
        ohm_dbus_signal_t   *dbus_signals;

	OhmPluginPrivate *priv;
};

struct _OhmPluginClass
{
	GObjectClass	parent_class;
};

GType		 ohm_plugin_get_type			(void);
OhmPlugin	*ohm_plugin_new				(void);

gboolean	 ohm_plugin_load			(OhmPlugin      *plugin,
							 const gchar	*name);

const gchar	*ohm_plugin_get_name			(OhmPlugin	*plugin);
const gchar	*ohm_plugin_get_version			(OhmPlugin	*plugin);
const gchar	*ohm_plugin_get_author			(OhmPlugin	*plugin);
gboolean	 ohm_plugin_notify			(OhmPlugin      *plugin,
							 const char	*key,
							 gint		 id,
							 gint		 value);

gboolean	 ohm_plugin_initialize			(OhmPlugin      *plugin);

ohm_method_t *ohm_plugin_exports (OhmPlugin *plugin);
ohm_method_t *ohm_plugin_imports (OhmPlugin *plugin);

G_END_DECLS

#endif /* __OHM_PLUGIN_INTERNAL_H */
