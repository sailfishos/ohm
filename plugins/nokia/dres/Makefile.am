plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_dres.la
libohm_dres_la_SOURCES = dres.c
libohm_dres_la_LIBADD = @OHM_PLUGIN_LIBS@	\
                        @LIBDRES_LIBS@		\
                        @LIBFACT_LIBS@
libohm_dres_la_LDFLAGS = -module -avoid-version
libohm_dres_la_CFLAGS = @OHM_PLUGIN_CFLAGS@	\
			@SWIPROLOG_CFLAGS@	\
                        @LIBDRES_CFLAGS@	\
			-DDEFAULT_DRES_DIR=\"${pkglibdir}/dres\" \
                        @LIBFACT_CFLAGS@
