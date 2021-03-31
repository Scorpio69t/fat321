MAKEFLAGS  += --no-print-directory

CC			= gcc
LD			= ld
CFLAGS		= -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
LDFLAGS		= -T ../scripts/kernel.lds --whole-archive
TARGET=kernel.bin
srctree	:= $(CURDIR)

export CC LD CFLAGS

subdir := boot core lib mm
HEADER := ./include

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

.PHONY: all iso clean qemu debug ttt $(subdir)

_all: all

-include Makefile.dep

all: $(TARGET)

$(TARGET): $(subdir)
	$(LD) $(LDFLAGS) -o $@ $(shell find $(subdir) -name "*.o")

$(subdir):
	@$(MAKE) -C $@

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: main.c
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep

iso: all
	grub-mkrescue -o alphaz.iso iso/

clean:
	find $(subdir) -name "*.o" -or -name "*.dep" | xargs rm
	test -e $(TARGET) && rm $(TARGET)

qemu:
	qemu-system-i386 -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G

debug:
	@qemu-system-i386 -s -S -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G &
	@gdb -x scripts/gdbinit
	@kill $$(ps | grep qemu | awk '{print $$1 }')
