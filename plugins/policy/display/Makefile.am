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
Data_DATA = display

lib_LTLIBRARIES = libohm_display.la
libohm_displayincludedir = $(includedir)/ohm
libohm_display_la_SOURCES = ohm-plugin-display.c
libohm_display_la_LIBADD = @HAL_LIBS@ @DBUS_LIBS@ $(INTLLIBS)
libohm_display_la_LDFLAGS = -module -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

clean-local:
	rm -f *.la
	rm -f *~
