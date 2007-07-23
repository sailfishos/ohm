EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = powerstatus.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_powerstatus.la
libohm_powerstatus_la_SOURCES = ohm-plugin-powerstatus.c
libohm_powerstatus_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_powerstatus_la_LDFLAGS = -module -avoid-version
libohm_powerstatus_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
