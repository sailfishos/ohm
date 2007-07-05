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
Data_DATA = backlight

lib_LTLIBRARIES = libohm_backlight.la
libohm_backlightincludedir = $(includedir)/ohm
libohm_backlight_la_SOURCES = ohm-plugin-backlight.c
libohm_backlight_la_LIBADD = @HAL_LIBS@ @DBUS_LIBS@ $(INTLLIBS)
libohm_backlight_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
