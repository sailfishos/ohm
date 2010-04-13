EXTRA_DIST = $(aegis_DATA)
aegisdir   = ../debian
aegis_DATA = ohm.aegis

CLEANFILES = $(aegis_DATA)

ohm.aegis: $(shell ls ./*.credentials)
	../scripts/ohm-aegis-config.sh $(shell pwd) > $@
