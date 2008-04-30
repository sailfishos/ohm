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
    gint value;

    ohm_debug("id: '%u', key: '%s'\n", id, key);

    /* setting an uninitialized key causes an error */

    if (strcmp(key, HEADPHONE_INSTALLED) == 0) {
        if (ohm_plugin_hal_get_int (plugin, id, HEADPHONE_INSTALLED, &value))
            ohm_plugin_conf_set_key (plugin, HEADPHONE_INSTALLED, value);
    }
    
    if (strcmp(key, HEADSET_TYPE) == 0) {
        if (ohm_plugin_hal_get_int (plugin, id, HEADSET_TYPE, &value))
            ohm_plugin_conf_set_key (plugin, HEADSET_TYPE, value);
    }

    return;
}

static void
plugin_initialize (OhmPlugin *plugin)
{
    guint num, i;
    gint  value;

	ohm_plugin_hal_init (plugin);
    ohm_plugin_hal_use_property_modified (plugin, hal_property_changed_cb);
    num = ohm_plugin_hal_add_device_capability (plugin, "headset");
	
    if (num == 0) {
        ohm_debug("No headset device(s) found.");
    }
    else {
        for (i = 0; i < num; i++) {
            /* set the initial values during the initialization */

            if (ohm_plugin_hal_get_int (plugin, i, HEADPHONE_INSTALLED, &value))
                ohm_plugin_conf_set_key (plugin, HEADPHONE_INSTALLED, value);

            if (ohm_plugin_hal_get_int (plugin, i, HEADSET_TYPE, &value))
                ohm_plugin_conf_set_key (plugin, HEADSET_TYPE, value);
        }

        ohm_debug("%i headset devices found", num);
    }

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
