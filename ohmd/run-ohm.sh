#!/bin/sh

OHM_TMPDIR=/tmp/run-ohmd-$USER

if [ "$1" = "--help" ]; then
	echo "Usage:"
	echo "  $0 [run-options] [ohm-options]"
	echo
	echo "Run Options:"
	echo "  --skip-plugin-install: Don't do a temporary install of the plugins."
	echo "                         Only use this if $0 has already been run"
	echo "                         without this option"
	echo "  --debug:               Run with gdb"
	echo "  --memcheck:            Run with valgrind memcheck tool"
	echo "  --massif:              Run with valgrind massif heap-profiling tool"
	echo
	./ohmd --help
	exit 0
fi

if [ "$1" = "--skip-plugin-install" ] ; then
	shift
else
	make -C ../etc install DESTDIR=$OHM_TMPDIR prefix=/
	make -C ../plugins install DESTDIR=$OHM_TMPDIR prefix=/
fi

if [ "$1" = "--debug" ] ; then
	shift
	prefix="gdb --args"
elif [ "$1" = "--memcheck" ] ; then
	shift
	prefix="valgrind --show-reachable=yes --leak-check=full --tool=memcheck"
elif [ "$1" = "--massif" ] ; then
	shift
	prefix="valgrind --tool=massif"
fi

export OHM_CONF_DIR=$OHM_TMPDIR/etc/ohm
export OHM_PLUGIN_DIR=$OHM_TMPDIR/lib/ohm

echo "Execing: $prefix ./ohmd --no-daemon --verbose $@"
sudo $prefix ./ohmd --no-daemon --verbose $@
