# top makefile

SUBDIR := shell app

.PHONY: default all $(SUBDIR) config kernel

default: all

kernel:
	cd src && $(MAKE)

all: config kernel $(SUBDIR)

clean:
	cd src && $(MAKE) $@
	$(foreach dir, $(SUBDIR), $(MAKE) clean -C $(dir);)

$(SUBDIR):
	$(MAKE) -C $@

bochs:
	./scripts/mkimg.sh
	bochs -f config/bochsrc

qemu:
	./scripts/mkimg.sh
	qemu-system-x86_64 -curses -drive file=feng.img,format=raw,index=0,media=disk -m 1G

config:
	ln -fsn $(CURDIR)/src/include/libc $(CURDIR)/include
	ln -fsn $(CURDIR)/src/libc $(CURDIR)/libc
