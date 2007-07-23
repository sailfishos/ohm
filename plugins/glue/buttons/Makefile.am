EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = buttons.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_buttons.la
libohm_buttons_la_SOURCES = ohm-plugin-buttons.c
libohm_buttons_la_LIBADD = @OHM_PLUGIN_LIBS@
libohm_buttons_la_LDFLAGS = -module -avoid-version
libohm_buttons_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
