
CC = gcc
LD = ld
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
LDFLAGS := -T ../scripts/proc.lds
HEADER:=../include
LIBS := ../libc/libc.a

targets := hello ls cat a mkdir
SOURCE := $(wildcard *.c)

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

.PHONY: default clean

default: $(targets)

-include Makefile.dep

%.o:%.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

%:%.o
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep

clean:
	rm -f $(targets)
	rm -f *.o
