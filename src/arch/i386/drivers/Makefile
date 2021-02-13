
HEADER := ../../../include
OBJS := console.o i8042.o vbe.o

-include Makefile.dep

default: $(OBJS)

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: *.c
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
