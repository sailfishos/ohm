INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = buttons.ini

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_buttons.la
libohm_buttonsincludedir = $(includedir)/ohm
libohm_buttons_la_SOURCES = ohm-plugin-buttons.c
libohm_buttons_la_LIBADD = $(INTLLIBS)
libohm_buttons_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
