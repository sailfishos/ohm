INCLUDES =							\
	$(HAL_CFLAGS)						\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GTHREAD_CFLAGS)					\
	-DDBUS_API_SUBJECT_TO_CHANGE				\
	-DVERSION_MAJOR=$(VERSION_MAJOR)			\
	-DVERSION_MINOR=$(VERSION_MINOR)			\
	-DVERSION_PATCH=$(VERSION_PATCH)			\
	-DBINDIR=\"$(bindir)\"					\
	-DLIBDIR=\"$(libdir)\"					\
	-DSBINDIR=\"$(sbindir)\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"				\
	-DDATADIR=\"$(datadir)\"				\
	-I$(top_builddir)/include

ohmpluginincludedir=$(includedir)/ohm

ohmplugininclude_HEADERS =

noinst_PROGRAMS = ohmd-test

ohmd_test_SOURCES = 						\
	ohm-debug.c						\
	ohm-debug.h						\
	ohm-conf.c						\
	ohm-conf.h						\
	ohm-marshal.h						\
	ohm-marshal.c						\
	ohm-test.c
ohmd_test_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(GMODULE_LIBS)

lib_LTLIBRARIES = libohmplugin.la libohmfact.la

libohmplugin_la_SOURCES =                                       \
	ohm-plugin.c

libohmplugin_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

libohmfact_la_SOURCES =                                         \
	ohm-fact.h 						\
	ohm-factstore.h						\
	ohm-factstore.c

libohmfact_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

sbin_PROGRAMS = ohmd

ohmd_SOURCES = 							\
	ohm-common.h						\
	ohm-debug.c						\
	ohm-debug.h						\
	ohm-conf.c						\
	ohm-conf.h						\
	ohm-marshal.h						\
	ohm-marshal.c						\
	ohm-keystore.c						\
	ohm-keystore.h						\
	ohm-module.c						\
	ohm-module.h						\
	ohm-plugin-internal.h					\
	ohm-manager.c						\
	ohm-manager.h						\
	ohm-dbus.c						\
	ohm-main.c

ohmd_LDADD = libohmplugin.la libohmfact.la                      \
        $(HAL_LIBS) $(DBUS_LIBS) $(GTHREAD_LIBS) $(GMODULE_LIBS)

BUILT_SOURCES =							\
	ohm-marshal.c						\
	ohm-marshal.h						\
	ohm-dbus-manager.h					\
	ohm-dbus-keystore.h

ohm-marshal.c: ohm-marshal.list
	echo "#include \"ohm-marshal.h\"" > $@ && \
	@GLIB_GENMARSHAL@ $< --prefix=ohm_marshal --body >> $@

ohm-marshal.h: ohm-marshal.list
	@GLIB_GENMARSHAL@ $< --prefix=ohm_marshal --header > $@

ohm-dbus-manager.h: ohm-dbus-manager.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=ohm_manager --mode=glib-server --output=ohm-dbus-manager.h $(srcdir)/ohm-dbus-manager.xml

ohm-dbus-keystore.h: ohm-dbus-keystore.xml
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=ohm_keystore --mode=glib-server --output=ohm-dbus-keystore.h $(srcdir)/ohm-dbus-keystore.xml

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST =							\
	ohm-marshal.list					\
	ohm-dbus-manager.xml					\
	ohm-dbus-keystore.xml
