plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_resolver.la
libohm_resolver_la_SOURCES = resolver.c
libohm_resolver_la_LIBADD = @OHM_PLUGIN_LIBS@	\
                        @LIBDRES_LIBS@		\
                        @LIBFACT_LIBS@
libohm_resolver_la_LDFLAGS = -module -avoid-version
libohm_resolver_la_CFLAGS = @OHM_PLUGIN_CFLAGS@	\
			@SWIPROLOG_CFLAGS@	\
                        @LIBDRES_CFLAGS@	\
                        @LIBFACT_CFLAGS@
