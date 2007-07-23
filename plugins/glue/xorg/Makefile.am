EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = xorg.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_xorg.la
libohm_xorg_la_SOURCES = ohm-plugin-xorg.c
libohm_xorg_la_LIBADD = @OHM_PLUGIN_LIBS@ @OHM_X11_LIBS@
libohm_xorg_la_LDFLAGS = -module -avoid-version
libohm_xorg_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
