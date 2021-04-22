
HEADER:=../../include/libc
SOURCE := $(wildcard *.c)
OBJS := main.o cache.o
LIBS := ../../libc/libc.a
LDFLAGS := -T ../../../scripts/proc.lds

ext2.mod: $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
