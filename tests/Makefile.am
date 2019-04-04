AM_CPPFLAGS =	\
	$(GLIB_CFLAGS) \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DVERSION_PATCH=$(VERSION_PATCH) \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_builddir)/include


test_fact_SOURCES   = test-fact.c
test_fact_LDADD     = $(top_builddir)/libfactstore/libohmfact.la $(GLIB_LIBS) -lcheck

noinst_PROGRAMS = test-fact

clean-local:
	rm -f *~

CLEANFILES = $(BUILT_SOURCES)


