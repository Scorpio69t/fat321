MAKEFLAGS  += -rR --no-print-directory

DEBUG = 1

CC			= gcc
LD			= ld
CFLAGS		= -c -O2 -Wall -fno-builtin -fno-common -fno-stack-protector -m32 -Wa,--32
LDFLAGS		= -T ../scripts/kernel.lds --whole-archive -m elf_i386
TARGET=kernel.bin
srctree	:= $(CURDIR)

# cpu platform
ARCH		?= i386
export ARCH CC LD CFLAGS

subdir := boot drivers fs init core lib mm

ifeq ($(DEBUG), 1)
	CFLAGS += -g -ggdb
endif

.PHONY: all iso clean qemu debug ttt $(subdir)

_all: all

all: $(TARGET)

$(TARGET): $(subdir)
	$(LD) $(LDFLAGS) -o $@ $(shell find . -name "*.o")

$(subdir):
	@$(MAKE) -C $@

iso: all
	grub-mkrescue -o alphaz.iso iso/

clean:
	find . -name "*.o" -or -name "*.dep" -or -name $(TARGET) | xargs rm

qemu:
	qemu-system-i386 -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G

debug:
	@qemu-system-i386 -s -S -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G &
	@gdb -x scripts/gdbinit
	@kill $$(ps | grep qemu | awk '{print $$1 }')
