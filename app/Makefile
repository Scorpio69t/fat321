
CC = gcc
LD = ld
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
SUBDIR := hello

export CC LD CFLAGS

.PHONY: clean $(SUBDIR)

default: $(SUBDIR)

$(SUBDIR):
	@$(MAKE) -C $@

clean:
	$(foreach dir, $(SUBDIR), $(MAKE) clean -C $(dir);)
