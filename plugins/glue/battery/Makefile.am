EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = battery.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_battery.la
libohm_battery_la_SOURCES = ohm-plugin-battery.c
libohm_battery_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_battery_la_LDFLAGS = -module -avoid-version
libohm_battery_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
