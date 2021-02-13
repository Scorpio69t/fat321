
HEADER := ../../../include

-include Makefile.dep

boot.o: boot.S
	$(CC) -I$(HEADER) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

Makefile.dep: *.S
	$(CC) -I$(HEADER) -MM $^ > Makefile.dep