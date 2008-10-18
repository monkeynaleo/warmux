#!/usr/bin/make -f

po_status: $(BRANCH)/po/wormux.pot
	./$(BRANCH)/tools/gettext/genstats $(BRANCH)/po $(BRANCH)/po/wormux.pot po_status/$(BRANCH)

$(BRANCH)/po/wormux.pot:
	$(MAKE) -C $(BRANCH)/po wormux.pot

last_update:
	cd $(BRANCH); LC_ALL=C svn up > ../po_status/last_update
	LC_ALL=C date >> po_status/last_update
	sed -i "s/\./ - /" po_status/last_update
