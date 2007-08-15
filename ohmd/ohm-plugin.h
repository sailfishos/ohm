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

G_BEGIN_DECLS

typedef struct _OhmPlugin OhmPlugin;
typedef struct _OhmPluginDesc OhmPluginDesc;
typedef struct _OhmPluginKeyIdMap OhmPluginKeyIdMap;

struct _OhmPluginKeyIdMap {
	const char	*key_name;
	gint		local_key_id;
};

typedef enum {
	OHM_LICENSE_LGPL,
	OHM_LICENSE_GPL,
	OHM_LICENSE_MIT,
	OHM_LICENSE_BSD,
	OHM_LICENSE_NON_FREE,
	OHM_LICENSE_FREE_OTHER
} OhmLicenseType;

/**
 * OhmPluginDesc:
 * @description: Plugin description
 * @version: Plugin version
 * @author: Plugin author
 * @license: Plugin license type
 * @initialize: method to call on plugin initialization
 * @destroy: method to call on plugin destruction
 * @notify: method to call to notify of key changes, using the id's described by
 *          #OHM_PLUGIN_INTERESTED
 * @padding: Padding for future expansion
 */
struct _OhmPluginDesc {
	const char		*description;
	const char		*version;
	const char		*author;
	OhmLicenseType	license;
	void		(*initialize)			(OhmPlugin *plugin);
	void		(*destroy)			(OhmPlugin *plugin);
	void		(*notify)			(OhmPlugin *plugin, gint id, gint value);
	gpointer padding[8];
};

#define OHM_PLUGIN_DESCRIPTION(description, version, author, license, initialize, destroy, notify) \
	G_MODULE_EXPORT const OhmPluginDesc ohm_plugin_desc = { \
		description, \
		version, \
		author, \
		license,\
		initialize, \
		destroy, \
		notify, \
		{0} \
	}

#define OHM_PLUGIN_INTERESTED(...) \
	G_MODULE_EXPORT const OhmPluginKeyIdMap ohm_plugin_interested[] = {__VA_ARGS__, {NULL,0}}

#define OHM_PLUGIN_PROVIDES(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_provides[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_REQUIRES(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_requires[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_SUGGESTS(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_suggests[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_PREVENTS(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_prevents[] = {__VA_ARGS__,NULL}

typedef void (*OhmPluginHalPropMod)			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key);
typedef void (*OhmPluginHalCondition)			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*name,
							 const gchar	*detail);

/* used by plugin to do crazy stuff */
gboolean	 ohm_plugin_spawn_async			(OhmPlugin      *plugin,
							 const gchar	*commandline);

/* used by plugin to manager */
gboolean	 ohm_plugin_conf_get_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		*value);
gboolean	 ohm_plugin_conf_set_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		 value);
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

G_END_DECLS

#endif /* __OHM_PLUGIN_H */
