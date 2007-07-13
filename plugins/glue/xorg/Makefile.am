INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = xorg.ini

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_xorg.la
libohm_xorgincludedir = $(includedir)/ohm
libohm_xorg_la_SOURCES = ohm-plugin-xorg.c
libohm_xorg_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)	$(OHM_X11_LIBS)

libohm_xorg_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
