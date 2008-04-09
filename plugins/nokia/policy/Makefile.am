plugindir = @OHM_PLUGIN_DIR@
plugin_LTLIBRARIES = libohm_policy.la
libohm_policy_la_SOURCES = policy.c
libohm_policy_la_LIBADD = @OHM_PLUGIN_LIBS@ \
                          @LIBPOLICY_LIBS@ \
                          @LIBSET_LIBS@ \
                          @LIBRELATION_LIBS@
libohm_policy_la_LDFLAGS = -module -avoid-version
libohm_policy_la_CFLAGS = @OHM_PLUGIN_CFLAGS@ \
                          @LIBPOLICY_CFLAGS@ \
                          @LIBSET_CFLAGS@ \
                          @LIBRELATION_CFLAGS@ \
			  -DDEFAULT_PROLOG_DIR=\"${pkglibdir}/prolog\"
