SUBDIRS = 				\
	include				\
	ohmd				\
	po				\
	initscript			\
	libohm				\
	libidletime			\
	tools				\
	etc				\
	plugins
#	docs


pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = ohm.pc ohmd/libohmplugin.pc ohmd/libohmfact.pc

dbusdir = @DBUS_SYS_DIR@
dist_dbus_DATA = ohm.conf

EXTRA_DIST =				\
	$(dbusconf_DATA)		\
	intltool-update.in		\
	intltool-merge.in		\
	intltool-extract.in		\
	ohm.pc.in			\
	ohm.conf.in

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

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --with-xauth-dir=/home/user
