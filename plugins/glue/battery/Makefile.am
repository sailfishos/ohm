INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = battery

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_battery.la
libohm_batteryincludedir = $(includedir)/ohm
libohm_battery_la_SOURCES = ohm-plugin-battery.c
libohm_battery_la_LIBADD = $(INTLLIBS)
libohm_battery_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
