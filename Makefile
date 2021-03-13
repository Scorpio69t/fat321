# top makefile

default: _all

_all:
	cd src && $(MAKE) $@

bochs:
	./scripts/mkiso.sh
	bochs -f config/bochsrc