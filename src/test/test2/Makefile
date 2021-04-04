
HEADER:=../../include/libc
SOURCE := $(wildcard *.c)
OBJS := test2.o
LIBS := ../../libc/libc.a
LDFLAGS := -T ../../../scripts/proc.lds

test2.mod: $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
