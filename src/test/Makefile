
subdir := test1 test2

.PHONY: default $(subdir)

default: $(subdir)

$(subdir):
	@$(MAKE) -C $@
