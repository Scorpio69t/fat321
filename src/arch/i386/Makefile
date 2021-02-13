subdir := lib kernel drivers boot

.PHONY: default $(subdir)

default: $(subdir)

$(subdir):
	@$(MAKE) -C $@
