INCLUDES =							\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GTHREAD_CFLAGS)					\
	-DDBUS_API_SUBJECT_TO_CHANGE				\
	-DBINDIR=\"$(bindir)\"					\
	-DLIBDIR=\"$(libdir)\"					\
	-DSBINDIR=\"$(sbindir)\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"				\
	-DDATADIR=\"$(datadir)\"

ohmpluginincludedir=$(includedir)/ohm

ohmplugininclude_HEADERS =					\
	ohm-plugin.h

noinst_PROGRAMS = ohmd-test

ohmd_test_SOURCES = 						\
	ohm-debug.c						\
	ohm-debug.h						\
	ohm-confobj.c						\
	ohm-confobj.h						\
	ohm-conf.c						\
	ohm-conf.h						\
	ohm-marshal.h						\
	ohm-marshal.c						\
	ohm-test.c
ohmd_test_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(GMODULE_LIBS)

sbin_PROGRAMS = ohmd

ohmd_SOURCES = 							\
	ohm-common.h						\
	ohm-debug.c						\
	ohm-debug.h						\
	ohm-confobj.c						\
	ohm-confobj.h						\
	ohm-conf.c						\
	ohm-conf.h						\
	ohm-marshal.h						\
	ohm-marshal.c						\
	ohm-keystore.c						\
	ohm-keystore.h						\
	ohm-module.c						\
	ohm-module.h						\
	ohm-plugin.c						\
	ohm-plugin.h						\
	ohm-manager.c						\
	ohm-manager.h						\
	ohm-main.c
ohmd_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(GMODULE_LIBS)

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
	libtool --mode=execute dbus-binding-tool --prefix=ohm_manager --mode=glib-server --output=ohm-dbus-manager.h $(srcdir)/ohm-dbus-manager.xml

ohm-dbus-keystore.h: ohm-dbus-keystore.xml
	libtool --mode=execute dbus-binding-tool --prefix=ohm_keystore --mode=glib-server --output=ohm-dbus-keystore.h $(srcdir)/ohm-dbus-keystore.xml

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST =							\
	ohm-marshal.list					\
	ohm-dbus-manager.xml					\
	ohm-dbus-keystore.xml
