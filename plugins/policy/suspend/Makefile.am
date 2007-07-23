EXTRA_DIST = $(Data_DATA)
Datadir = @OHM_PLUGIN_CONF_DIR@
Data_DATA = suspend.ini

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_suspend.la
libohm_suspend_la_SOURCES = ohm-plugin-suspend.c
libohm_suspend_la_LIBADD = @HAL_LIBS@ @OHM_PLUGIN_LIBS@
libohm_suspend_la_LDFLAGS = -module -avoid-version
libohm_suspend_la_CFLAGS = @HAL_CFLAGS@ @OHM_PLUGIN_CFLAGS@
