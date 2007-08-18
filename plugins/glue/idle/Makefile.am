LOCAL_LIBIDLETIME_LIBS = $(top_builddir)/libidletime/libidletime.la
LOCAL_LIBIDLETIME_CFLAGS = -I$(top_srcdir)/libidletime

plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_idle.la
libohm_idle_la_SOURCES = ohm-plugin-idle.c
libohm_idle_la_LIBADD = @OHM_PLUGIN_LIBS@ $(LOCAL_LIBIDLETIME_LIBS)
libohm_idle_la_LDFLAGS = -module -avoid-version
libohm_idle_la_CFLAGS = @OHM_PLUGIN_CFLAGS@ $(LOCAL_LIBIDLETIME_CFLAGS)
