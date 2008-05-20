plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_prolog.la
libohm_prolog_la_SOURCES = prolog.c
libohm_prolog_la_LIBADD = @OHM_PLUGIN_LIBS@ \
                          @LIBPROLOG_LIBS@
libohm_prolog_la_LDFLAGS = -module -avoid-version
libohm_prolog_la_CFLAGS = @OHM_PLUGIN_CFLAGS@ \
                          @LIBPROLOG_CFLAGS@ \
			  -DDEFAULT_PROLOG_DIR=\"${pkglibdir}/prolog\"
