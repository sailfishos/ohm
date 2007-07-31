#!/bin/sh

OHM_TMPDIR=/tmp/run-ohmd-$USER

if [ "$1" = "--skip-plugin-install" ] ; then
	shift
else
	make -C ../etc install DESTDIR=$OHM_TMPDIR prefix=/
	make -C ../plugins install DESTDIR=$OHM_TMPDIR prefix=/
fi

if [ "$1" = "--debug" ] ; then
	shift
	commandline="sudo gdb --args ./ohmd --no-daemon --verbose $@"
else
	commandline="sudo ./ohmd --no-daemon --verbose $@"
fi

export OHM_CONF_DIR=$OHM_TMPDIR/etc/ohm
export OHM_PLUGIN_DIR=$OHM_TMPDIR/lib/ohm

$commandline
