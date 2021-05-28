
HEADER:=../../include/libc
SOURCE := $(wildcard *.c)
OBJS := main.o
LIBS := ../../libc/libc.a
LDFLAGS := -T ../../../scripts/proc.lds

init.srv: $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
