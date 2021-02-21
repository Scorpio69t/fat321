
HEADER := ../../include
OBJS := console.o i8042.o vbe.o

default: $(OBJS)

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

Makefile.dep: *.c
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
