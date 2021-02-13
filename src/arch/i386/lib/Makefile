
.PHONY: default dep

HEADER := ../../../include
SOURCE := $(wildcard *.c) $(wildcard *.S)
OBJS := $(patsubst %.c, %.o, $(SOURCE))

-include Makefile.dep

default: $(OBJS)

%.o: %.S
	$(CC) -I$(include) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -I$(include) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(include) -MM $^ > Makefile.dep
