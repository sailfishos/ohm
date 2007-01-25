INCLUDES = \
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"			\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

EXTRA_DIST =							\
	libohm-marshal.list

lib_LTLIBRARIES=libohm.la

libohmincludedir=$(includedir)/ohm

libohminclude_HEADERS =						\
	libohm.h

libohm_la_SOURCES =						\
	libohm-marshal.h					\
	libohm-marshal.c					\
	libohm.c						\
	libohm.h

libohm_la_LIBADD = @DBUS_LIBS@ $(INTLLIBS)

libohm_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

BUILT_SOURCES = 						\
	libohm-marshal.c					\
	libohm-marshal.h

libohm-marshal.c: libohm-marshal.list
	echo "#include \"libohm-marshal.h\"" > $@ && \
	@GLIB_GENMARSHAL@ $< --prefix=libohm_marshal --body >> $@

libohm-marshal.h: libohm-marshal.list
	@GLIB_GENMARSHAL@ $< --prefix=libohm_marshal --header > $@

clean-local:
	rm -f *~
	rm -f libohm-marshal.c libohm-marshal.h

CLEANFILES = $(BUILT_SOURCES)
