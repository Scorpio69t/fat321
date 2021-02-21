
.PHONY: default dep

HEADER := ../../include
SOURCE := $(wildcard *.c) $(wildcard *.S)
OBJS := $(patsubst %.c, %.o, $(SOURCE))

default: $(OBJS)

-include Makefile.dep

%.o: %.S
	$(CC) -I$(HEADER) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
