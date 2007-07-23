EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = dpms.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_dpms.la
libohm_dpms_la_SOURCES = ohm-plugin-dpms.c
libohm_dpms_la_LIBADD = @OHM_PLUGIN_LIBS@ @OHM_X11_LIBS@
libohm_dpms_la_LDFLAGS = -module -avoid-version
libohm_dpms_la_CFLAGS = @OHM_PLUGIN_CFLAGS@
