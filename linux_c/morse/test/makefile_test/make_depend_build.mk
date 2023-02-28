BUILDDIR=dependencies-build
TARGETS=$(BUILDDIR)/1.out $(BUILDDIR)/2.out

.phony: all
all: $(TARGETS)

$(BUILDDIR)/1.out: 1.in $(BUILDDIR)
	touch $@

$(BUILDDIR)/2.out: 2.in $(BUILDDIR)
	touch $@

$(BUILDDIR):
	mkdir $(BUILDDIR)

clean: 
	rm -rf 1.in 2.in $(BUILDDIR)
