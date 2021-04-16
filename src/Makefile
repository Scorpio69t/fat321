MAKEFLAGS  += --no-print-directory

CC			= gcc
LD			= ld
AR 			= ar
CFLAGS		= -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
ARFLAGS		= -rc
LDFLAGS		= -T ../scripts/kernel.lds --whole-archive
TARGET=kernel.bin
srctree	:= $(CURDIR)

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

export CC LD CFLAGS KCFLAGS ARFLAGS

kendir := boot core mm
libdir := libc
moddir := init drivers fs
HEADER := ./include

ifdef DEBUG
	moddir += test
endif

.PHONY: all iso clean qemu debug $(kendir) $(libdir) $(moddir)

_all: all

-include Makefile.dep

all: $(libdir) $(TARGET) $(moddir)

$(TARGET): $(kendir)
	$(LD) $(LDFLAGS) -o $@ $(shell find $(kendir) -name "*.o")

$(kendir):
	@$(MAKE) -C $@

$(libdir):
	@$(MAKE) -C $@

$(moddir):
	@echo "\nstart make $@"
	@$(MAKE) -C $@

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

clean:
	find . -name "*.o" | xargs -r rm
	find . -name "*.dep" | xargs -r rm
	find . -name "*.a" | xargs -r rm
	find . -name "*.mod" | xargs -r rm
	rm -f $(TARGET)
