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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <glib/gi18n.h>
#include <gmodule.h>

#include "ohm-module.h"
#include "ohm-plugin.h"

#define OHM_MODULE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), OHM_TYPE_MODULE, OhmModulePrivate))

struct OhmModulePrivate
{
	GSList			*module_require;
	GSList			*module_suggest;
	GSList			*module_prevent;
	GSList			*module_loaded;
};

G_DEFINE_TYPE (OhmModule, ohm_module, G_TYPE_OBJECT)

/**
 * ohm_module_get_key:
 **/
gboolean
ohm_module_require (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("require '%s'", name);
	return TRUE;
}

/**
 * ohm_module_add_notify_key:
 **/
gboolean
ohm_module_suggest (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("suggest '%s'", name);
	return TRUE;
}

/**
 * ohm_module_set_key:
 *
 **/
gboolean
ohm_module_prevent (OhmModule   *module,
		    const gchar *name)
{
	g_debug ("prevent '%s'", name);
	return TRUE;
}

/**
 * ohm_module_finalize:
 **/
static void
ohm_module_finalize (GObject *object)
{
	OhmModule *module;
	g_return_if_fail (object != NULL);
	g_return_if_fail (OHM_IS_MODULE (object));
	module = OHM_MODULE (object);

	g_return_if_fail (module->priv != NULL);
	G_OBJECT_CLASS (ohm_module_parent_class)->finalize (object);
}

/**
 * ohm_module_class_init:
 **/
static void
ohm_module_class_init (OhmModuleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	   = ohm_module_finalize;

	g_type_class_add_private (klass, sizeof (OhmModulePrivate));
}

/**
 * ohm_module_init:
 **/
static void
ohm_module_init (OhmModule *module)
{
	module->priv = OHM_MODULE_GET_PRIVATE (module);
	/* clear lists */
	module->priv->module_require = NULL;
	module->priv->module_suggest = NULL;
	module->priv->module_prevent = NULL;
	module->priv->module_loaded = NULL;

	/* play for now */
	OhmPlugin *plugin;
	plugin = ohm_plugin_load ("libpluginbattery.so");
	ohm_plugin_unload (plugin);
}

/**
 * ohm_module_new:
 **/
OhmModule *
ohm_module_new (void)
{
	OhmModule *module;
	module = g_object_new (OHM_TYPE_MODULE, NULL);
	return OHM_MODULE (module);
}
