INCLUDES =							\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GTHREAD_CFLAGS)					\
	-DDBUS_API_SUBJECT_TO_CHANGE				\
	-DBINDIR=\"$(bindir)\"					\
	-DSBINDIR=\"$(sbindir)\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"				\
	-DDATADIR=\"$(datadir)\"

sbin_PROGRAMS = ohmd

ohmd_SOURCES = 							\
	ohm-common.h						\
	ohm-module.c						\
	ohm-module.h						\
	ohm-plugin.c						\
	ohm-plugin.h						\
	ohm-keystore.c						\
	ohm-keystore.h						\
	ohm-conf.c						\
	ohm-conf.h						\
	ohm-manager.c						\
	ohm-manager.h						\
	ohm-main.c
ohmd_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(GMODULE_LIBS)

BUILT_SOURCES =							\
	ohm-dbus-manager.h					\
	ohm-dbus-keystore.h

ohm-dbus-manager.h: ohm-dbus-manager.xml
	libtool --mode=execute dbus-binding-tool --prefix=ohm_manager --mode=glib-server --output=ohm-dbus-manager.h $(srcdir)/ohm-dbus-manager.xml

ohm-dbus-keystore.h: ohm-dbus-keystore.xml
	libtool --mode=execute dbus-binding-tool --prefix=ohm_keystore --mode=glib-server --output=ohm-dbus-keystore.h $(srcdir)/ohm-dbus-keystore.xml

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST =							\
	ohm-dbus-manager.xml					\
	ohm-dbus-keystore.xml
