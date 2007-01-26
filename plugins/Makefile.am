SUBDIRS = 				\
	idle				\
	battery				\
	acadapter			\
	timeremaining			\
	powerstatus			\
	backlight

if HAVE_DPMS_EXTENSION
SUBDIRS += dpms
endif

INCLUDES =							\
	-I$(top_srcdir)/ohmd					\
	$(GLIB_CFLAGS)						\
	$(DBUS_CFLAGS)						\
	$(GMODULE_CFLAGS)					\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

clean-local:
	rm -f *~
