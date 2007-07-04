INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = timeremaining

lib_LTLIBRARIES = libohm_timeremaining.la
libohm_timeremainingincludedir = $(includedir)/ohm
libohm_timeremaining_la_SOURCES = ohm-plugin-timeremaining.c
libohm_timeremaining_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)
libohm_timeremaining_la_LDFLAGS = -module -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

clean-local:
	rm -f *.la
	rm -f *~
