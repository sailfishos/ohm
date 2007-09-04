SUBDIRS = reference

IMAGE_FILES =				\
	ohm-logo.png			\
	ohm-logo.svg			\
	ohm-logo-small.png		\
	ohm-dbus-access.png		\
	ohm-dbus-access.svg		\
	ohm-structure.png		\
	ohm-structure.svg		\
	ohm-sessions.png		\
	ohm-sessions.svg

SPEC_XML_FILES =			\
	index.xml			\
	introduction.xml		\
	plugins.xml			\
	faq.xml				\
	dbus-interface.xml

SPEC_HTML_FILES =			\
	index.html

clean-local:
	rm -f *~
	rm -f *.html

EXTRA_DIST = 				\
	index.xml.in			\
	config.xsl			\
	docbook.css			\
	$(SPEC_XML_FILES)		\
	$(IMAGE_FILES)

if DOCBOOK_DOCS_ENABLED

htmldocdir = $(docdir)
htmldoc_DATA = index.html

index.html : introduction.xml
	$(XMLTO) html-nochunks -m config.xsl index.xml

EXTRA_DIST += $(SPEC_HTML_FILES)

endif #DOCBOOK_DOCS_ENABLED
