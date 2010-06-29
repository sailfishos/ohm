PACKAGE := ohm
VERSION := 0.1.2
THISDIR := meego

TOPDIR  := $(shell while [ ! -d $(THISDIR) -a "`pwd`" != "/" ]; do cd ..; done;\
		   [ -d $(THISDIR) ] && pwd || exit 1)

ARCH     := i386
RPM      := $(PACKAGE)-$(VERSION).$(ARCH).rpm
SPECFILE := $(TOPDIR)/$(THISDIR)/$(PACKAGE).spec
TARBALL  := $(TOPDIR)/$(THISDIR)/$(PACKAGE)-$(VERSION).tar.gz
TARDIR  := $(notdir $(TOPDIR))

STAMP   := $(shell date +%Y-%m-%d-%H%M%S)
TMPDIR  := /tmp/$(PACKAGE)-$(USER)/$(STAMP)


all: rpms cleanup

# rule to build RPM(s) for this package
rpms: rmtar $(SPECFILE) $(TARBALL)
	mkdir -p $(TMPDIR)/{BUILD,RPMS,SOURCES,SPECS} && \
	cp $(SPECFILE) $(TMPDIR)/SPECS && \
	cp $(TARBALL) $(TMPDIR)/SOURCES && \
	rpmbuild --define '_topdir $(TMPDIR)' \
	    -bb $(TMPDIR)/SPECS/$(PACKAGE).spec && \
	cp $(TMPDIR)/RPMS/$(ARCH)/*.rpm $(TOPDIR)/$(THISDIR)

cleanup:
	rm -fr $(TMPDIR) && \
	rm -f $(SPECFILE)
	rm -f $(TARBALL)

# rules to patch the SPEC file and build a source tarball
$(SPECFILE): $(SPECFILE).in
	cat $< | \
	    sed 's/##__VERSION__##/$(VERSION)/g;s/##__TARDIR__##/$(TARDIR)/g'| \
	    sed 's/##__ARCH__##/$(ARCH)/g' \
	    > $@

$(TARBALL):
	pushd $(TOPDIR) && make distclean || : && ./autogen.sh && popd && \
	pushd $(TOPDIR)/.. && \
	tar -cvzf /tmp/$(notdir $@) \
	    --exclude '*/$(THISDIR)' \
	    --exclude '*/.git' $(notdir $(TARDIR)) && \
	mv /tmp/$(notdir $@) $@

meego-export: $(SPECFILE) $(TARBALL)

rmtar:
	rm -f $(TARBALL)
