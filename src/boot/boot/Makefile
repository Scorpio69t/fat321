
HEADER := ../../include

default: multiboot.o boot.o

-include Makefile.dep

%.o: %.c
	$(CC) -I$(HEADER) $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -I$(HEADER) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

Makefile.dep: *.S *.c
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep
