
.PHONY: default dep

HEADER := ../include
SOURCE := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SOURCE))

default: $(OBJS)

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep

debug:
	echo $(OBJS)