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
	echo "  --time:                Time OHMd startup"
	echo "  --debug:               Run with gdb"
	echo "  --memcheck:            Run with valgrind memcheck tool"
	echo "  --massif:              Run with valgrind massif heap-profiling tool"
	echo "  --efence:              Run with electric-fence chedking for overruns"
	echo "  --underfence:          Run with electric-fence checking for underruns"
	echo
	./ohmd --help
	exit 0
fi

if [ "$1" = "--skip-plugin-install" ] ; then
	shift
else
	rm -rf $OHM_TMPDIR
	make -C ../etc install DESTDIR=$OHM_TMPDIR prefix=/
	make -C ../plugins install DESTDIR=$OHM_TMPDIR prefix=/
fi

extra='--no-daemon --verbose --g-fatal-critical'

if [ "$1" = "--time" ] ; then
	shift
	prefix="time -p"
	extra="--no-daemon --timed-exit"
elif [ "$1" = "--debug" ] ; then
	shift
	prefix="gdb --args"
elif [ "$1" = "--memcheck" ] ; then
	shift
	prefix="valgrind --show-reachable=yes --leak-check=full --tool=memcheck --suppressions=./valgrind.supp $VALGRIND_EXTRA"
	export G_SLICE="always-malloc"
elif [ "$1" = "--massif" ] ; then
	shift
	prefix="valgrind --tool=massif --suppressions=./valgrind.supp $VALGRIND_EXTRA"
	export G_SLICE="always-malloc"
elif [ "$1" = "--efence" ] ; then
	shift
	prefix="gdb -x ./efence.gdb --args"
	export G_SLICE="always-malloc"
elif [ "$1" = "--underfence" ] ; then
	shift
	prefix="gdb -x ./underfence.gdb --args"
	export G_SLICE="always-malloc"
fi

export OHM_CONF_DIR=$OHM_TMPDIR/etc/ohm
export OHM_PLUGIN_DIR=$OHM_TMPDIR/lib/ohm

echo "Execing: $prefix ./ohmd $extra $@"
sudo $prefix ./ohmd $extra $@
