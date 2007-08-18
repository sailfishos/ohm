EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = timeouts.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_timeouts.la
libohm_timeouts_la_SOURCES = timeouts.c
libohm_timeouts_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_timeouts_la_LDFLAGS = -module -avoid-version
libohm_timeouts_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
