# build an executable named myprog from myp
CC=gcc
CFLAGS=-I.
DEPS = radio.h

%.o: %.c: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
make: make radio.o radio_test.o
	$(CC) -o radio.c radio_test.c
clean: 
	$(RM) radio radio_test
