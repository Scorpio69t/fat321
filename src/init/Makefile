
.PHONY: default dep

HEADER := ../include
SOURCE := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SOURCE))

-include Makefile.dep

default: $(OBJS)

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
