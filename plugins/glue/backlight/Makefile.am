EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = backlight.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_backlight.la
libohm_backlight_la_SOURCES = ohm-plugin-backlight.c
libohm_backlight_la_LIBADD = @HAL_LIBS@ @OHM_PLUGIN_LIBS@
libohm_backlight_la_LDFLAGS = -module -avoid-version
libohm_backlight_CFLAGS = @HAL_CFLAGS@ @OHM_PLUGIN_CFLAGS@
