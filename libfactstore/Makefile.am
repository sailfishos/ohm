INCLUDES =	\
	$(GLIB_CFLAGS) \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DVERSION_PATCH=$(VERSION_PATCH) \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_builddir)/include

lib_LTLIBRARIES = libohmfact.la

libohmfact_la_SOURCES = \
	ohm-fact.h  \
	ohm-factstore.h \
	ohm-factstore.c

libohmfact_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

test_fact_SOURCES   = test-fact.c
test_fact_LDADD     = libohmfact.la $(GLIB_LIBS)

noinst_PROGRAMS = test-fact

clean-local:
	rm -f *~

CLEANFILES = $(BUILT_SOURCES)

