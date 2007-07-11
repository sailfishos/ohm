INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = powerstatus.ini

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_powerstatus.la
libohm_powerstatusincludedir = $(includedir)/ohm
libohm_powerstatus_la_SOURCES = ohm-plugin-powerstatus.c
libohm_powerstatus_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)
libohm_powerstatus_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
