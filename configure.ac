AC_PREREQ(2.61)

AC_INIT([ohm], m4_esyscmd([./git-version-gen .tarball-version]))
AC_CONFIG_SRCDIR([ohmd/ohm-main.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

m4_define(version_major, `echo $VERSION | cut -d- -f1 | cut -d. -f1`)
m4_define(version_minor, `echo $VERSION | cut -d- -f1 | cut -d. -f1`)
m4_define(version_patch, `echo $VERSION | cut -d- -f1 | cut -d. -f1`)

AC_SUBST(VERSION)
AC_SUBST(VERSION_MAJOR, version_major)
AC_SUBST(VERSION_MINOR, version_minor)
AC_SUBST(VERSION_PATCH, version_patch)
AC_SUBST(VERSION_FULL, version_major.version_minor.version_patch)

# libtool versioning - this applies to libohm
#
# See http://sources.redhat.com/autobook/autobook/autobook_91.html#SEC91 for details
#
LT_CURRENT=2
LT_REVISION=0
LT_AGE=0
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL
#AC_PROG_INTLTOOL([0.33.0])
#AM_GLIB_GNU_GETTEXT
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

AM_PROG_CC_C_O

# GTK_DOC_CHECK([1.8])

#ALL_LINGUAS="en_GB"
#AC_SUBST(ALL_LINGUAS)

dnl ---------------------------------------------------------------------------
dnl - gettext stuff
dnl ---------------------------------------------------------------------------
#AC_DEFINE(GETTEXT_PACKAGE, "AC_PACKAGE_NAME", [foo])
#GETTEXT_PACKAGE=AC_PACKAGE_NAME
#AC_SUBST(GETTEXT_PACKAGE)

dnl ---------------------------------------------------------------------------
dnl - Extra verbose warning switches
dnl ---------------------------------------------------------------------------
if test "$GCC" = "yes"; then
    CPPFLAGS="$CPPFLAGS -Wall -Wcast-align -Wno-uninitialized -fno-strict-aliasing"
    #CPPFLAGS="$CPPFLAGS -Wall"
fi

dnl # Default to meego (avoids having to touch all branches right now)
with_distro=meego
AC_ARG_WITH(distro, AC_HELP_STRING([--with-distro=DISTRO], [Specify the Linux distribution to target: One of redhat, gentoo, debian, slackware, maemo, meego, auto, or none]))
if test "x$with_distro" = "xauto"; then
	AC_CHECK_FILE(/etc/meego-release,with_distro="meego")
	AC_CHECK_FILE(/etc/mandrake-release,with_distro="mandrake")
	AC_CHECK_FILE(/etc/redhat-release,with_distro="redhat")
	AC_CHECK_FILE(/etc/fedora-release,with_distro="redhat")
	AC_CHECK_FILE(/etc/gentoo-release,with_distro="gentoo")
	AC_CHECK_FILE(/etc/debian_version,with_distro="debian")
	AC_CHECK_FILE(/etc/slackware-version,with_distro="slackware")
fi
with_distro=`echo ${with_distro} | tr '[[:upper:]]' '[[:lower:]]' `

if test "x$with_distro" = "xauto"; then
         with_distro=""
fi
if test "z$with_distro" = "z"; then
	echo "Linux distribution autodetection failed, you must specify the distribution to target using --with-distro=DISTRO"
	exit 1
elif test "z$with_distro" = "znone"; then
	echo "No distro specific startup scripts will be built."
else
	case $with_distro in
		redhat|gentoo|debian|slackware|meego|maemo) ;;
		*)
			echo "Your distribution (${with_distro}) is not yet supported!  (patches welcome)"
			exit 1
			;;
	esac
fi
AM_CONDITIONAL(TARGET_REDHAT, test x"$with_distro" = xredhat)
AM_CONDITIONAL(TARGET_GENTOO, test x"$with_distro" = xgentoo)
AM_CONDITIONAL(TARGET_DEBIAN, test x"$with_distro" = xdebian)
AM_CONDITIONAL(TARGET_MANDRAKE, test x"$with_distro" = xmandrake)
AM_CONDITIONAL(TARGET_SLACKWARE, test x"$with_distro" = xslackware)
AM_CONDITIONAL(TARGET_MAEMO, test x"$with_distro" = xmaemo)
AM_CONDITIONAL(TARGET_MEEGO, test x"$with_distro" = xmeego)

PKG_CHECK_MODULES(GLIB, glib-2.0 gobject-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)


PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= 0.70 dbus-1 >= 0.70)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)


PKG_CHECK_MODULES(LIBTRACE, libsimple-trace >= 0.0.0)
AC_SUBST(LIBTRACE_CFLAGS)
AC_SUBST(LIBTRACE_LIBS)

AC_SUBST(OHM_CONF_DIR, "\$(sysconfdir)/ohm")

AC_SUBST(OHM_PLUGIN_CONF_DIR, "\$(OHM_CONF_DIR)/plugins.d")
AC_SUBST(OHM_PLUGIN_DIR, "\$(libdir)/ohm")
AC_SUBST(OHM_PLUGIN_CFLAGS, "-I\$(top_srcdir)/ohmd -I\$(top_srcdir)/include $GLIB_CFLAGS $DBUS_CFLAGS $GMODULE_CFLAGS")
AC_SUBST(OHM_PLUGIN_LIBS, "$GLIB_LIBS $DBUS_LIBS $GMODULE_LIBS $LIBTRACE_LIBS")

dnl Assume single-user-device for now
single_user_device="yes"

if test "$single_user_device" = yes; then
	AC_DEFINE(OHM_SINGLE_USER_DEVICE, 1, [Whether OHM is built for a single-user device with a known .Xauthority location])

	AC_ARG_WITH(xauth,
		[AC_HELP_STRING([--without-xauth],
			[Whether xauth is needed to access the Xserver.]
			)],
		[with_xauth="$withval"], [with_xauth="yes"]
		)


	AC_ARG_WITH(xauth-dir,
		[AC_HELP_STRING([--with-xauth-dir=<dir>],
			[directory where we can find .Xauthority (for single-user device)]
			)]
		)

	if test "x$with_xauth" = "xno"; then
		AC_DEFINE(OHM_DEVICE_NO_XAUTH, 1, [No Xauth needed for Xserver access])
	elif test -z "$with_xauth_dir"; then
		AC_MSG_ERROR([Must use --without-xauth or --with-xauth-dir for a single-user device])
	else
		AC_DEFINE_UNQUOTED(OHM_DEVICE_XAUTH_DIR, "$with_xauth_dir", [Where to find .Xauthority])
	fi
fi

dnl ---------------------------------------------------------------------------
dnl - Where should we put documentation ?
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(doc-dir,
              [AC_HELP_STRING([--with-doc-dir=<dir>],
              [directory to install documentation])])
if ! test -z "$with_doc_dir"; then
	DOCDIR="$with_doc_dir/ohm-$VERSION"
else
	DOCDIR="\$(docdir\)"
fi
AC_SUBST(DOCDIR)

dnl ---------------------------------------------------------------------------
dnl - DocBook Documentation
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(docbook-docs, [  --enable-docbook-docs   build documentation (requires xmlto)],enable_docbook_docs=$enableval,enable_docbook_docs=auto)
AC_PATH_PROG(XMLTO, xmlto, no)
AC_MSG_CHECKING([whether to build DocBook documentation])
if test x$XMLTO = xno ; then
    have_docbook=no
else
    have_docbook=yes
fi
if test x$enable_docbook_docs = xauto ; then
    if test x$have_docbook = xno ; then
        enable_docbook_docs=no
    else
        enable_docbook_docs=yes
    fi
fi
if test x$enable_docbook_docs = xyes; then
    if test x$have_docbook = xno; then
	AC_MSG_ERROR([Building DocBook docs explicitly required, but DocBook not found])
    fi
fi
AM_CONDITIONAL(DOCBOOK_DOCS_ENABLED, test x$enable_docbook_docs = xyes)
AC_MSG_RESULT(yes)

AC_ARG_WITH(dbus-sys, [  --with-dbus-sys=<dir>   where D-BUS system.d directory is])

if ! test -z "$with_dbus_sys" ; then
    DBUS_SYS_DIR="$with_dbus_sys"
else
    DBUS_SYS_DIR="\${sysconfdir}/dbus-1/system.d"
fi
AC_SUBST(DBUS_SYS_DIR)
AC_DEFINE_UNQUOTED(DBUS_SYSTEMD_DIR, "$DBUS_SYS_DIR", [Where system.d dir for DBUS is])


dnl ---------------------------------------------------------------------------
dnl - Some utility functions to make checking for X things easier.
dnl ---------------------------------------------------------------------------
# Like AC_CHECK_HEADER, but it uses the already-computed -I directories.
AC_DEFUN([AC_CHECK_X_HEADER], [
  ac_save_CPPFLAGS="$CPPFLAGS"
  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_CHECK_HEADER([$1],[$2],[$3],[$4])
  CPPFLAGS="$ac_save_CPPFLAGS"])

# Like AC_TRY_COMPILE, but it uses the already-computed -I directories.
AC_DEFUN([AC_TRY_X_COMPILE], [
  ac_save_CPPFLAGS="$CPPFLAGS"
  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_TRY_COMPILE([$1], [$2], [$3], [$4])
  CPPFLAGS="$ac_save_CPPFLAGS"])

# Like AC_CHECK_LIB, but it uses the already-computed -I and -L directories.
# Use this sparingly; it probably doesn't work very well on X programs.
AC_DEFUN([AC_CHECK_X_LIB], [
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LDFLAGS="$LDFLAGS"
#  ac_save_LIBS="$LIBS"
  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  # note: $X_CFLAGS includes $x_includes
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  if test \! -z "$libdir" ; then
    LDFLAGS="$LDFLAGS -L$libdir"
  fi
  # note: $X_LIBS includes $x_libraries
  LDFLAGS="$LDFLAGS $ALL_X_LIBS"
  AC_CHECK_LIB([$1], [$2], [$3], [$4], [$5])
  CPPFLAGS="$ac_save_CPPFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"
#  LIBS="$ac_save_LIBS"
  ])

# Usage: HANDLE_X_PATH_ARG([variable_name],
#                          [--command-line-option],
#                          [descriptive string])
#
# All of the --with options take three forms:
#
#   --with-foo (or --with-foo=yes)
#   --without-foo (or --with-foo=no)
#   --with-foo=/DIR
#
# This function, HANDLE_X_PATH_ARG, deals with the /DIR case.  When it sees
# a directory (string beginning with a slash) it checks to see whether
# /DIR/include and /DIR/lib exist, and adds them to $X_CFLAGS and $X_LIBS
# as appropriate.
AC_DEFUN([HANDLE_X_PATH_ARG], [
   case "$[$1]" in
    yes) ;;
    no)  ;;
    /*)
     AC_MSG_CHECKING([for [$3] headers])
     d=$[$1]/include
     if test -d $d; then
       X_CFLAGS="-I$d $X_CFLAGS"
       AC_MSG_RESULT($d)
     else
       AC_MSG_RESULT(not found ($d: no such directory))
     fi
     AC_MSG_CHECKING([for [$3] libs])
     d=$[$1]/lib
     if test -d $d; then
       X_LIBS="-L$d $X_LIBS"
       AC_MSG_RESULT($d)
     else
       AC_MSG_RESULT(not found ($d: no such directory))
     fi
     # replace the directory string with "yes".
     [$1]_req="yes"
     [$1]=$[$1]_req
     ;;
    *)
     echo ""
     echo "error: argument to [$2] must be \"yes\", \"no\", or a directory."
     echo "       If it is a directory, then \`DIR/include' will be added to"
     echo "       the -I list, and \`DIR/lib' will be added to the -L list."
     exit 1
     ;;
   esac
  ])


dnl ---------------------------------------------------------------------------
dnl - Check for the DPMS server extension.
dnl ---------------------------------------------------------------------------

have_dpms=no
with_dpms_req=unspecified
OHM_X11_LIBS="-lresolv"
AC_ARG_WITH(dpms-ext,
[  --with-dpms-ext         Include support for the DPMS extension.],
  [with_dpms="$withval"; with_dpms_req="$withval"],[with_dpms=yes])

HANDLE_X_PATH_ARG(with_dpms, --with-dpms-ext, DPMS)

if test "$with_dpms" = yes; then

  # first check for dpms.h
  AC_CHECK_X_HEADER(X11/extensions/dpms.h, [have_dpms=yes],,
                    [#include <X11/Xlib.h>
		     #include <X11/Xmd.h>])
  # if that succeeded, then check for the DPMS code in the libraries
  if test "$have_dpms" = yes; then
    # first look in -lXext (this is where it is with XFree86 4.0)
    have_dpms=no
    AC_CHECK_X_LIB(Xext, DPMSInfo, [have_dpms=yes], [true], -lXext -lX11)
    # if that failed, look in -lXdpms (this is where it was in XFree86 3.x)
    if test "$have_dpms" = no; then
      AC_CHECK_X_LIB(Xdpms, DPMSInfo,
                    [have_dpms=yes; XDPMS_LIBS="-lXdpms"], [true],
                    -lXext -lX11)
    fi
  fi
  # if that succeeded, then we've really got it.
  if test "$have_dpms" = yes; then
    AC_DEFINE(HAVE_DPMS_EXTENSION, 1, [Define if the DPMS extension is available])
    OHM_X11_LIBS="$OHM_X11_LIBS -lXext"
  fi
  AM_CONDITIONAL(HAVE_DPMS_EXTENSION, test "$have_dpms" = yes, [DPMS extension is available])
elif test "$with_dpms" != no; then
  echo "error: must be yes or no: --with-dpms-ext=$with_dpms"
  exit 1
fi

AC_SUBST(OHM_X11_LIBS)


dnl -----------------------------------
dnl - Profiling support using gprof.
dnl -----------------------------------
AC_ARG_ENABLE(gprof,
              [  --enable-gprof       enable profiling with gprof],
              enable_gprof=$enableval,enable_gprof=no)
AC_MSG_CHECKING([for profiling (gprof) support])
if test x$enable_gprof = xyes ; then
    AC_MSG_RESULT(yes)
    CFLAGS="-pg $CFLAGS"
    LDFLAGS="-pg $LDFLAGS"
else
    AC_MSG_RESULT(no)
fi

dnl -----------------------------------
dnl - OSSO mlocknice support
dnl -----------------------------------
AC_ARG_ENABLE(mlocknice,
              [  --enable-mlocknice     enable OSSO mlocknice support],
              enable_mlocknice=$enableval,enable_mlocknice=auto)

if test x$enable_mlocknice = xauto; then
    AC_CHECK_HEADERS([mlocknice.h], [mlocknice=yes], [mlocknice=no])
elif test x$enable_mlocknice = xyes; then
    AC_CHECK_HEADERS([mlocknice.h],
                     [mlocknice=yes],
                     [AC_MSG_ERROR([OSSO mlocknice not available])])
else
    mlocknice=no
fi

if test x$mlocknice = xyes; then
    AC_MSG_NOTICE([OSSO mlocknice support enabled])
    LDFLAGS="$LDFLAGS -lmlocknice"
fi


dnl -----------------------------------
dnl - Legacy OHM plugins
dnl -----------------------------------
AC_ARG_WITH([legacy],
            [AS_HELP_STRING([--with-legacy],
              [enable legacy OHM plugins])],
            [],
            [with_legacy=no])

AS_IF([test "x$with_legacy" != xno], [enable_legacy=yes], [enable_legacy=no])

AM_CONDITIONAL(BUILD_LEGACY, test x$enable_legacy = xyes)

if test x$enable_legacy = xyes;
then
  AC_DEFINE([BUILD_LEGACY], 1, [Build legacy plugins ?])
  AC_MSG_NOTICE([Legacy OHM plugins WILL BE built])

  PKG_CHECK_MODULES(GDK, gdk-2.0)
  AC_SUBST(GDK_CFLAGS)
  AC_SUBST(GDK_LIBS)
else
  AC_MSG_NOTICE([Legacy OHM plugins WILL NOT be built])
fi

AC_SUBST(BUILD_LEGACY)


dnl --------------------------------
dnl - keystore support
dnl --------------------------------
AC_ARG_ENABLE(keystore,
              [  --enable-keystore     enable keystore support],
              enable_keystore=$enableval,enable_keystore=no)

if test x$enable_gthread = xauto; then
    if test x$enable_legacy = xyes; then
        enable_keystore=yes
    else
        enable_keystore=no
    fi
fi

if test x$enable_keystore = xyes; then
    PKG_CHECK_MODULES(KEYSTORE, keystore-2.0)

    AC_DEFINE([HAVE_KEYSTORE], 1, [Build keystore support ?])
    AC_MSG_NOTICE([keystore support WILL BE built])
else
    AC_MSG_NOTICE([keystore support WILL NOT be built])
fi

AM_CONDITIONAL(HAVE_KEYSTORE, test x$enable_keystore = xyes)
AC_SUBST(KEYSTORE_CFLAGS)
AC_SUBST(KEYSTORE_LIBS)
AC_SUBST(HAVE_KEYSTORE)


dnl ------------------------------------
dnl - gthread support
dnl ------------------------------------
AC_ARG_ENABLE(gthread,
              [  --enable-gthread     enable gthread support],
              enable_gthread=$enableval,enable_gthread=auto)

if test x$enable_gthread = xauto; then
    if test x$enable_legacy = xyes; then
        enable_gthread=yes
    else
        enable_gthread=no
    fi
fi

if test x$enable_gthread = xyes; then
    PKG_CHECK_MODULES(GTHREAD, gthread-2.0)

    AC_DEFINE([HAVE_GTHREAD], 1, [Build gthread support ?])
    AC_MSG_NOTICE([gthread support WILL BE built])
else
    AC_MSG_NOTICE([gthread support WILL NOT be built])
fi

AM_CONDITIONAL(HAVE_GTHREAD, test x$enable_gthread = xyes)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)
AC_SUBST(HAVE_GTHREAD)

PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= 0.70 dbus-1 >= 0.70)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)


dnl ------------------------------------
dnl - HAL support
dnl ------------------------------------
AC_ARG_ENABLE(hal,
              [  --enable-hal     enable HAL support],
              enable_hal=$enableval,enable_hal=auto)

if test x$enable_hal = xauto; then
    if test x$enable_legacy = xyes; then
        enable_hal=yes
    else
        enable_hal=no
    fi
fi

if test x$enable_hal = xyes; then
    PKG_CHECK_MODULES(HAL, hal >= 0.5.7)

    if pkg-config --atleast-version=0.5.10 hal; then
        AC_DEFINE(HAL_SET_BRIGHTNESS_UNSIGNED, 1, [org.freedesktop.Hal.Device.LaptopPanel.SetBrightness returns UINT32])
    fi

    AC_DEFINE([HAVE_HAL], 1, [Build hal support ?])
    AC_MSG_NOTICE([HAL support will be built])
else
    AC_MSG_NOTICE([HAL support will not be built])
fi

AM_CONDITIONAL(HAVE_HAL, test x$enable_hal = xyes)
AC_SUBST(HAL_CFLAGS)
AC_SUBST(HAL_LIBS)
AC_SUBST(HAVE_HAL)


dnl -----------------------------------
dnl - Miscallaneous OHM tools (old keystore commandline stuff)
dnl -----------------------------------
AC_ARG_ENABLE(tools,
              [  --enable-tools    enable miscallaneous OHM tools],
              enable_tools=$enableval,enable_tools=no)

AM_CONDITIONAL(BUILD_TOOLS, test x$enable_tools = xyes)

if test x$enable_tools = xyes;
then
  AC_DEFINE([BUILD_TOOLS], 1, [Build miscallaneous OHM tools ?])
  AC_MSG_NOTICE([Miscallaneous OHM tools WILL BE built])
else
  AC_MSG_NOTICE([Miscallaneous OHM tools WILL NOT be built])
fi

AC_SUBST(BUILD_TOOLS)


dnl -----------------------------------
dnl - aegis support
dnl -----------------------------------
AC_ARG_ENABLE(aegis,
              [  --enable-aegis     enable aegis support],
              enable_aegis=$enableval,enable_aegis=auto)

if test x$enable_aegis = xauto; then
    AC_CHECK_FILES([/usr/bin/aegis-deb-add], [aegis=yes], [aegis=no])
elif test x$enable_aegis = xyes; then
    AC_CHECK_FILES([/usr/bin/aegis-deb-add],
                     [aegis=yes],
                     [AC_MSG_ERROR([aegis-deb-add (aegis-builder) not available])])
else
    aegis=no
fi

AM_CONDITIONAL(AEGIS_SUPPORT, test x$aegis = xyes)

if test x$aegis = xyes; then
    AC_DEFINE([AEGIS_SUPPORT], 1, [Support for aegis ?])
    AC_MSG_NOTICE([aegis support enabled])
fi

AC_SUBST(AEGIS_SUPPORT)

AC_OUTPUT([
ohm.conf
Makefile
etc/Makefile
aegis/Makefile
include/Makefile
libfactstore/Makefile
ohmd/Makefile
ohmd/libohmplugin.pc
ohmd/libohmfact.pc
plugins/Makefile
plugins/policy/Makefile
plugins/policy/display/Makefile
plugins/policy/suspend/Makefile
plugins/policy/timeouts/Makefile
plugins/glue/Makefile
plugins/glue/acadapter/Makefile
plugins/glue/backlight/Makefile
plugins/glue/battery/Makefile
plugins/glue/buttons/Makefile
plugins/glue/dpms/Makefile
plugins/glue/idle/Makefile
plugins/glue/powerstatus/Makefile
plugins/glue/timeremaining/Makefile
plugins/glue/xorg/Makefile
plugins/glue/xrandr/Makefile
libidletime/Makefile
libohm/Makefile
tools/Makefile
tests/Makefile
initscript/Makefile
initscript/RedHat/Makefile
initscript/Gentoo/Makefile
initscript/Slackware/Makefile
initscript/MeeGo/Makefile
])


echo
echo Distribution targeting: ${with_distro}
echo 'if this is not correct, please specifiy your distro with --with-distro=DISTRO'

dnl ==========================================================================
echo "
                    Open Hardware Manager $VERSION
                  ==============================

        prefix:                    ${prefix}
        libdir:                    ${libdir}
        datadir:                   ${datadir}
        documentation dir:         ${docdir}
        compiler:                  ${CC}
        cflags:                    ${CFLAGS}
        DPMS support:              ${have_dpms}
        Docbook support:           ${enable_docbook_docs}
        Gtk-doc support:           ${enable_gtk_doc}
        Profiling support:         ${enable_gprof}
        OSSO mlockince support:    ${mlocknice}
	Legacy OHM plugins:        ${enable_legacy}
	Legacy OHM tools:          ${enable_tools}
	Aegis support:             ${aegis}
"
