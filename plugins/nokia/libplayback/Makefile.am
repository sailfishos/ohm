plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_libplayback.la
libohm_libplayback_la_SOURCES = libplayback.c
libohm_libplayback_la_LIBADD = @OHM_PLUGIN_LIBS@	\
                               @LIBFACT_LIBS@
libohm_libplayback_la_LDFLAGS = -module -avoid-version
libohm_libplayback_la_CFLAGS = @OHM_PLUGIN_CFLAGS@	\
                               @LIBFACT_CFLAGS@
