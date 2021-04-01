
HEADER:= ../../include .
OBJS := disk.o
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector
LDFLAGS := -T ../../../scripts/proc.lds

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

.PHONY: _all clean

_all: disk.out

disk.out: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

clean:
	find . -name "*.o" -or -name "*.out" | xargs rm
