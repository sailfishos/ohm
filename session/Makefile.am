INCLUDES = \
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	-I$(top_srcdir) -I$(top_srcdir)/libohm			\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"			\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

noinst_PROGRAMS = session-daemon

session_daemon_SOURCES = 						\
	session-daemon.c
session_daemon_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(top_builddir)/libohm/libohm.la 

clean-local:
	rm -f *~
