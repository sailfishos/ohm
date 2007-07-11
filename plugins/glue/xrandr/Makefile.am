INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = xrandr.ini

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_xrandr.la
libohm_xrandrincludedir = $(includedir)/ohm
libohm_xrandr_la_SOURCES = ohm-plugin-xrandr.c
libohm_xrandr_la_LIBADD = $(INTLLIBS)
libohm_xrandr_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
