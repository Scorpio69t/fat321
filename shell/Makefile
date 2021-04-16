
CC = gcc
LD = ld
CFLAGS := -c -Wall -fno-builtin -fno-common -fno-stack-protector -mcmodel=large
LDFLAGS := -T ../scripts/proc.lds

HEADER:=../include
SOURCE := $(wildcard *.c)
OBJS := main.o
LIBS := ../libc/libc.a
TARGET := sh

.PHONY: clean

$(TARGET): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: $(SOURCE)
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep

clean:
	find . -name "*.o" | xargs -r rm
	find . -name "*.dep" | xargs -r rm
	rm -f $(TARGET)
