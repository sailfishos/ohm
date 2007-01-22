INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = powerstatus

lib_LTLIBRARIES = libpolicypowerstatus.la
libpolicypowerstatusincludedir = $(includedir)/ohm
libpolicypowerstatus_la_SOURCES = ohm-plugin-powerstatus.c
libpolicypowerstatus_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)
libpolicypowerstatus_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *~
