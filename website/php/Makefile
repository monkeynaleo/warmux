#!/usr/bin/make -f
#
# Update translation status
#

default: update

update: svn_update
	-rm -r po_status
	-rm last_update
	-$(MAKE) -C wormux clean
	$(MAKE) po_status
	$(MAKE) last_update
	lftp -f wormux.ftp

svn_update: wormux
	cd wormux; svn update

wormux:
	svn co svn://svn.gna.org/svn/wormux/trunk wormux

last_update:
	LC_ALL=C date > last_update

po_status: wormux/po/wormux.pot
	./wormux/tools/gettext/genstats wormux/po wormux/po/wormux.pot po_status

wormux/po/wormux.pot: wormux/po/Makefile
	$(MAKE) -C wormux/po wormux.pot

wormux/po/Makefile: wormux/po/Makefile.am
	cd wormux; \
	  ./autogen.sh; \
	  ./configure
