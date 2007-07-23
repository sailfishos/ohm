EXTRA_DIST = $(Data_DATA)
Datadir = $(OHM_PLUGIN_CONF_DIR)
Data_DATA = display.ini

plugindir = $(OHM_PLUGIN_DIR)
plugin_LTLIBRARIES = libohm_display.la
libohm_display_la_SOURCES = ohm-plugin-display.c
libohm_display_la_LIBADD = @HAL_LIBS@ @OHM_PLUGIN_LIBS@
libohm_display_la_LDFLAGS = -module -avoid-version
libohm_display_la_CFLAGS = @HAL_CFLAGS@ @OHM_PLUGIN_CFLAGS@
