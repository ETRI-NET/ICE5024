#SUBDIRS := lib cmd processmgr ipcmgr ribmgr polmgr ripd pifmgr lacpd
SUBDIRS    := lib cmd processmgr ipcmgr ribmgr polmgr ripd isisd ospfd

all:    .subdirs
clean:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean;)

.subdirs:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean; $(MAKE) -C $(d); $(MAKE) -C $(d) install;)

