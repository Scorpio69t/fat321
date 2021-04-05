
HEADER:=../../include/libc
SOURCE := $(wildcard *.c)
OBJS := vfs.o
LIBS := ../../libc/libc.a
LDFLAGS := -T ../../../scripts/proc.lds

vfs.mod: $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
