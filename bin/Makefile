
CC = gcc
LD = ld
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
LDFLAGS := -T ../scripts/proc.lds
HEADER:=../include
LIBS := ../libc/libc.a

targets := hello ls cat
SOURCE := $(wildcard *.c)

ifdef DEBUG
	CFLAGS += -g -ggdb
endif

.PHONY: default clean

default: $(targets)

%:%.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@.o $<
	$(LD) $(LDFLAGS) -o $@ $@.o $(LIBS)

-include Makefile.dep

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep

clean:
	rm -f $(targets)
