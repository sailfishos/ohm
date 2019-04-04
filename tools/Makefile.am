AM_CPPFLAGS = \
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	-I$(top_srcdir) -I$(top_srcdir)/libohm			\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"			\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

noinst_PROGRAMS = libohm-test
bin_PROGRAMS = lsohm ohm-set-key ohm-get-key

libohm_test_SOURCES = 						\
	libohm-test.c
libohm_test_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(top_builddir)/libohm/libohm.la 

ohm_set_key_SOURCES = 						\
	ohm-set-key.c
ohm_set_key_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(top_builddir)/libohm/libohm.la 

ohm_get_key_SOURCES = 						\
	ohm-get-key.c
ohm_get_key_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(top_builddir)/libohm/libohm.la 

lsohm_SOURCES = 						\
	lsohm.c
lsohm_LDADD = $(DBUS_LIBS) $(GTHREAD_LIBS) $(top_builddir)/libohm/libohm.la 

clean-local:
	rm -f *~
	rm -f libohm-marshal.c libohm-marshal.h
