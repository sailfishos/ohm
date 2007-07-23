EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = acadapter.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_acadapter.la
libohm_acadapter_la_SOURCES = ohm-plugin-acadapter.c
libohm_acadapter_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_acadapter_la_LDFLAGS = -module -avoid-version
libohm_acadapter_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
