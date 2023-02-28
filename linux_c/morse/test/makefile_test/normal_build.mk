BUILDDIR=mkdir-build
TARGETS=$(BUILDDIR)/1.out $(BUILDDIR)/2.out

.phony: all clean
all: $(TARGETS)

$(BUILDDIR)/1.out: 1.in
	mkdir -p $(BUILDDIR)
	touch $@

$(BUILDDIR)/2.out: 2.in
	mkdir -p $(BUILDDIR)
	touch $@

clean: 
	rm -rf 1.in 2.in $(BUILDDIR)
