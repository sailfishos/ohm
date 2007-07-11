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
Data_DATA = suspend.ini

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_suspend.la
libohm_suspendincludedir = $(includedir)/ohm
libohm_suspend_la_SOURCES = ohm-plugin-suspend.c
libohm_suspend_la_LIBADD = @HAL_LIBS@ @DBUS_LIBS@ $(INTLLIBS)
libohm_suspend_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
