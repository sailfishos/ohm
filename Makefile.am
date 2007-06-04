SUBDIRS = 				\
	ohmd				\
	po				\
	initscript			\
	docs				\
	libohm				\
	tools				\
	etc				\
	plugins

dbusconfdir = $(sysconfdir)/dbus-1/system.d
dbusconf_DATA = ohm.conf

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ohm.pc

dbusdir = $(DBUS_SYS_DIR)
dist_dbus_DATA = ohm.conf

EXTRA_DIST =				\
	$(dbusconf_DATA)		\
	intltool-update.in		\
	intltool-merge.in		\
	intltool-extract.in		\
	ohm.pc.in			\
	ohm.conf.in			\
	mkinstalldirs

DISTCLEANFILES = 			\
	intltool-extract		\
	intltool-merge			\
	intltool-update

MAINTAINERCLEANFILES =	\
	Makefile.in			\
	aclocal.m4			\
	config.guess			\
	config.h.in			\
	config.sub			\
	configure			\
	depcomp				\
	install-sh 			\
	libtool 			\
	ltmain.sh 			\
	missing 			\
	autom4te.cache/*		\
	po/$(GETTEXT_PACKAGE).pot	\
	po/*.bak
