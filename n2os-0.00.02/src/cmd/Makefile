SUBDIRS ?= ../3rd/libedit ../lib/cmdlib cmsh cm
all:	.subdirs
clean:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean || exit 1;)

.subdirs:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean || exit 1; $(MAKE) -C $(d) || exit 1; $(MAKE) -C $(d) install || exit 1;)
