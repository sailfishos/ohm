#include <gmodule.h>
#include <glib.h>
#include <ohm-plugin.h>
#include <ohm-debug.h>
#include <dbus/dbus-glib.h>
#include <string.h>

#define HEADPHONE_INSTALLED "headset.headphone_installed"
#define HEADSET_TYPE "headset.headset_type"

static void
hal_property_changed_cb (OhmPlugin   *plugin,
			 guint        id,
			 const gchar *key)
{
    guint value;

    ohm_debug("id: '%u', key: '%s'\n", id, key);

    if (strcmp(key, HEADPHONE_INSTALLED) == 0) {
        ohm_plugin_hal_get_int (plugin, id, HEADPHONE_INSTALLED, &value);
        ohm_plugin_conf_set_key (plugin, HEADPHONE_INSTALLED, value);
    }
    
    if (strcmp(key, HEADSET_TYPE) == 0) {
        ohm_plugin_hal_get_int (plugin, id, HEADSET_TYPE, &value);
        ohm_plugin_conf_set_key (plugin, HEADSET_TYPE, value);
    }

    return;
}

static void
plugin_initialize (OhmPlugin *plugin)
{
	ohm_plugin_hal_init (plugin);

	/* we want this function to get the property modified events for all devices */
	ohm_plugin_hal_use_property_modified (plugin, hal_property_changed_cb);

	/* hal capability: listens only to headset events */
	ohm_plugin_hal_add_device_capability (plugin, "headset");
    
    /* TODO: should set initial values during the initialization */

    return;
}

OHM_PLUGIN_DESCRIPTION (
	"headset",
	"0.0.1",
	"ismo.h.puustinen@nokia.com",
	OHM_LICENSE_NON_FREE,
	plugin_initialize,	/* initialize */
	NULL,	/* destroy */
	NULL	/* notify */
);

OHM_PLUGIN_PROVIDES(HEADPHONE_INSTALLED, HEADSET_TYPE);
