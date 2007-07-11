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

#ifndef __OHM_PLUGIN_H
#define __OHM_PLUGIN_H

#include <glib-object.h>

G_BEGIN_DECLS

#define OHM_TYPE_PLUGIN		(ohm_plugin_get_type ())
#define OHM_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), OHM_TYPE_PLUGIN, OhmPlugin))
#define OHM_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), OHM_TYPE_PLUGIN, OhmPluginClass))
#define OHM_IS_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), OHM_TYPE_PLUGIN))
#define OHM_IS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), OHM_TYPE_PLUGIN))
#define OHM_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), OHM_TYPE_PLUGIN, OhmPluginClass))

typedef struct OhmPluginPrivate OhmPluginPrivate;

typedef struct
{
	GObject		  parent;
	OhmPluginPrivate *priv;
} OhmPlugin;

typedef struct
{
	GObjectClass	parent_class;
	void		(* add_interested)		(OhmPlugin	*plugin,
							 const gchar	*key,
							 gint		 id);
	void		(* add_require)			(OhmPlugin	*plugin,
							 const gchar	*name);
	void		(* add_suggest)			(OhmPlugin	*plugin,
							 const gchar	*name);
	void		(* add_prevent)			(OhmPlugin	*plugin,
							 const gchar	*name);
	void		(* hal_key_changed)		(OhmPlugin	*plugin,
							 const gchar	*key);
} OhmPluginClass;

typedef struct {
	gchar		*description;
	gchar		*version;
	gchar		*author;
	gboolean	(*preload)			(OhmPlugin *plugin);
	void		(*unload)			(OhmPlugin *plugin);
	void		(*coldplug)			(OhmPlugin *plugin);
	void		(*conf_notify)			(OhmPlugin *plugin, gint id, gint value);
} OhmPluginInfo;

typedef void (*OhmPluginHalPropMod) 			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key);
typedef void (*OhmPluginHalCondition) 			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*name,
							 const gchar	*detail);


#define OHM_INIT_PLUGIN(plugininfo) G_MODULE_EXPORT OhmPluginInfo *ohm_init_plugin (OhmPlugin *plugin) {return &(plugin_info);}

GType		 ohm_plugin_get_type			(void);
OhmPlugin 	*ohm_plugin_new				(void);

gboolean	 ohm_plugin_preload			(OhmPlugin      *plugin,
							 const gchar	*name);

gboolean	 ohm_plugin_require			(OhmPlugin	*plugin,
							 const gchar	*name);
gboolean	 ohm_plugin_suggest			(OhmPlugin	*plugin,
							 const gchar	*name);
gboolean	 ohm_plugin_prevent			(OhmPlugin	*plugin,
							 const gchar	*name);

const gchar	*ohm_plugin_get_name			(OhmPlugin	*plugin);
const gchar	*ohm_plugin_get_version			(OhmPlugin	*plugin);
const gchar	*ohm_plugin_get_author			(OhmPlugin	*plugin);

/* used by plugin to do crazy stuff */
gboolean	 ohm_plugin_spawn_async			(OhmPlugin      *plugin,
							 const gchar	*commandline);

/* used by plugin to manager */
gboolean	 ohm_plugin_conf_provide		(OhmPlugin      *plugin,
							 const gchar	*name);
gboolean	 ohm_plugin_conf_get_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		*value);
gboolean	 ohm_plugin_conf_set_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		 value);
gboolean	 ohm_plugin_conf_interested		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		 id);
/* used by plugin for hal */
gboolean	 ohm_plugin_hal_init			(OhmPlugin	*plugin);
gboolean	 ohm_plugin_hal_use_property_modified	(OhmPlugin	*plugin,
							 OhmPluginHalPropMod func);
gboolean	 ohm_plugin_hal_use_condition		(OhmPlugin	*plugin,
							 OhmPluginHalCondition func);
gboolean	 ohm_plugin_hal_get_bool		(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key,
							 gboolean	*state);
gboolean	 ohm_plugin_hal_get_int			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key,
							 gint		*state);
gchar		*ohm_plugin_hal_get_udi			(OhmPlugin	*plugin,
							 guint		 id);
guint		 ohm_plugin_hal_add_device_capability	(OhmPlugin	*plugin,
							 const gchar	*capability);

/* used by manager to plugin */
gboolean	 ohm_plugin_conf_notify			(OhmPlugin      *plugin,
							 gint		 id,
							 gint		 value);
gboolean	 ohm_plugin_coldplug			(OhmPlugin      *plugin);

G_END_DECLS

#endif /* __OHM_PLUGIN_H */
