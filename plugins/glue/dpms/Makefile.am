INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST = $(Data_DATA)
Datadir = $(sysconfdir)/ohm/plugins
Data_DATA = dpms

lib_LTLIBRARIES = libohm_dpms.la
libohm_dpmsincludedir = $(includedir)/ohm
libohm_dpms_la_SOURCES = ohm-plugin-dpms.c
libohm_dpms_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)	$(OHM_X11_LIBS)

libohm_dpms_la_LDFLAGS = -module -avoid-version

clean-local:
	rm -f *.la
	rm -f *~
