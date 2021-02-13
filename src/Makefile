MAKEFLAGS  += -rR --no-print-directory

DEBUG = 1

CC			= gcc
LD			= ld
CFLAGS		= -c -O2 -Wall -fno-builtin -fno-common -fno-stack-protector
LDFLAGS		= -T ../scripts/kernel.lds --whole-archive
TARGET=kernel.bin
srctree	:= $(CURDIR)

# cpu platform
ARCH		?= i386
export ARCH CC LD CFLAGS

subdir := arch/$(ARCH) drivers fs init core lib mm

ifeq ($(ARCH), i386)
	CFLAGS += -m32 -Wa,--32
	LDFLAGS += -m elf_i386
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -g -ggdb
endif

.PHONY: all iso config clean qemu debug ttt $(subdir)

_all: all

all: config $(TARGET)

$(TARGET): $(subdir)
	$(LD) $(LDFLAGS) -o $@ $(shell find . -name "*.o")

$(subdir):
	@$(MAKE) -C $@

iso: all
	grub-mkrescue -o alphaz.iso iso/

# 编译前对整个项目的配置。包括相关目录的生成以及创建目录链接等工作。所有子目录的Makefile的导入
# 工作必须在此前完成。
config:
	@ln -fsn $(srctree)/include/asm-$(ARCH) include/asm

clean:
	find . -name "*.o" -or -name "*.dep" -or -name $(TARGET) | xargs rm

qemu:
	qemu-system-i386 -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G

debug:
	@qemu-system-i386 -s -S -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G &
	@gdb -x scripts/gdbinit
	@kill $$(ps | grep qemu | awk '{print $$1 }')
