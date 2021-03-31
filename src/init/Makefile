
CC := gcc
LD := ld
HEADER:=../include
OBJS := main.o entry.o
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector
LDFLAGS := -T ../../scripts/proc.lds

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

.PHONY: _all clean

_all: init.out

init.out: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -I$(HEADER) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

clean:
	find . -name "*.o" -or -name "*.out" | xargs rm
