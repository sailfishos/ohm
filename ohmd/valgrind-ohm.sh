#!/bin/sh

#valgrind --num-callers=20 --show-reachable=yes --leak-check=yes --tool=memcheck ./ohmd --no-daemon --timed-exit $@
valgrind --show-reachable=yes --tool=memcheck --leak-check=full ./ohmd --no-daemon --timed-exit $@
