INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(GDK_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-I$(top_srcdir)/libidletime				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"			\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

LOCAL_LIBIDLETIME_LIBS =					\
	$(top_builddir)/libidletime/libidletime.la

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = idle

plugindir = $(libdir)/ohm
plugin_LTLIBRARIES = libohm_idle.la
libohm_idleincludedir = $(includedir)/ohm
libohm_idle_la_SOURCES = ohm-plugin-idle.c
libohm_idle_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS) $(LOCAL_LIBIDLETIME_LIBS)
libohm_idle_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
