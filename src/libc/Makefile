
.PHONY: default dep

HEADER := ../include/libc
SOURCE := $(wildcard *.c) $(wildcard *.S)
OBJS := $(patsubst %.c, %.o, $(SOURCE))
OBJS := $(patsubst %.S, %.o, $(OBJS))

libc.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -I$(HEADER) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
