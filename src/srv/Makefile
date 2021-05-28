

subdir := init disk keyboard video ext2 vfs

.PHONY: default $(subdir)

default: $(subdir)

$(subdir):
	@$(MAKE) -C $@
