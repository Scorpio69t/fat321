# MAKEFLAGS  += -rR --no-print-directory

DEBUG = 1

.PHONY: all iso config clean qemu debug

CC			= gcc
LD			= ld
AR 			= ar
CFLAGS		= -c -O2 -Wall -fno-builtin -fno-common -fno-stack-protector
LDFLAGS		= -T ../scripts/kernel.lds --whole-archive
ARFLAGS		= -rc
TATGET=kernel.bin
srctree	:= $(CURDIR)

# cpu platform
ARCH		?= i386
export ARCH

export CC LD AR
export CFLAGS LDFLAGS ARFLAGS

subdir := arch/$(ARCH) dirvers fs init core lib mm

ifeq ($(ARCH), i386)
	CFLAGS += -m32 -Wa,--32
	LDFLAGS += -m elf_i386
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif

_all: all

all: config $(TATGET)

$(TATGET): $(subdir)
	OBJS=$(shell find . -name "*.o")
	$(LD) $(LDFLAGS) -o $@ $^

$(subdir):
	$(MAKE) -C $@

iso: all
	grub-mkrescue -o alphaz.iso iso/

# 编译前对整个项目的配置。包括相关目录的生成以及创建目录链接等工作。所有子目录的Makefile的导入
# 工作必须在此前完成。
config: $(dir $(src-all))
	@ln -fsn $(srctree)/include/asm-$(ARCH) include/asm
	
clean:
	rm -rf $(target)/kernel.bin

qemu:
	qemu-system-i386 -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G

debug:
	@qemu-system-i386 -s -S -cdrom alphaz.iso -m 2G,slots=3,maxmem=4G &
	@gdb -x scripts/gdbinit
	@kill $$(ps | grep qemu | awk '{print $$1 }')

