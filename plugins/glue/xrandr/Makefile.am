EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = xrandr.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_xrandr.la
libohm_xrandr_la_SOURCES = ohm-plugin-xrandr.c
libohm_xrandr_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_xrandr_la_LDFLAGS = -module -avoid-version
libohm_xrandr_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
