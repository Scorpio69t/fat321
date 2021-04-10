# top makefile

default: _all

_all:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@

bochs:
	./scripts/mkimg.sh
	bochs -f config/bochsrc

qemu:
	./scripts/mkimg.sh
	qemu-system-x86_64 -curses  -drive file=feng.img,format=raw,index=0,media=disk -m 1G
