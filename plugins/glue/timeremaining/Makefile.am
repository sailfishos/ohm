EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = timeremaining.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_timeremaining.la
libohm_timeremaining_la_SOURCES = ohm-plugin-timeremaining.c
libohm_timeremaining_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_timeremaining_la_LDFLAGS = -module -avoid-version
libohm_timeremaining_la_CFLAGS= @OHM_PLUGIN_CFLAGS@
