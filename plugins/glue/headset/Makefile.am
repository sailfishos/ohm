EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = headset.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_headset.la
libohm_headset_la_SOURCES = ohm-plugin-headset.c
libohm_headset_la_LIBADD = @HAL_LIBS@ @OHM_PLUGIN_LIBS@
libohm_headset_la_LDFLAGS = -module -avoid-version
libohm_headset_la_CFLAGS = @HAL_CFLAGS@ @OHM_PLUGIN_CFLAGS@
