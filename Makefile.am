SUBDIRS = 				\
	include				\
	libfactstore                    \
	ohmd				\
	initscript			\
	tests				\
	etc				
#	po				
#	docs

if BUILD_LEGACY
SUBDIRS += libidletime plugins
endif

if BUILD_TOOLS
SUBDIRS += libohm tools
endif

if AEGIS_SUPPORT
SUBDIRS += aegis
endif

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = ohm.pc ohmd/libohmplugin.pc ohmd/libohmfact.pc

dbusdir = @DBUS_SYS_DIR@
dist_dbus_DATA = ohm.conf

EXTRA_DIST =				\
	$(dbusconf_DATA)		\
	ohm.pc.in			\
	ohm.conf.in
#   intltool-update.in		
#	intltool-merge.in		
#	intltool-extract.in		

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

DISTCHECK_CONFIGURE_FLAGS=--disable-gtk-doc --with-xauth-dir=/home/user
