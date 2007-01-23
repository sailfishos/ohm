INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(HAL_CFLAGS)						\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = acadapter

lib_LTLIBRARIES = libohm_acadapter.la
libohm_acadapterincludedir = $(includedir)/ohm
libohm_acadapter_la_SOURCES = ohm-plugin-acadapter.c
libohm_acadapter_la_LIBADD = @HAL_LIBS@ @DBUS_LIBS@ $(INTLLIBS)
libohm_acadapter_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *~
