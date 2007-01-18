INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

lib_LTLIBRARIES=libpluginbattery.la
libpluginbatteryincludedir=$(includedir)/ohm
libpluginbattery_la_SOURCES = ohm-plugin-battery.c
libpluginbattery_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)
libpluginbattery_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *~
