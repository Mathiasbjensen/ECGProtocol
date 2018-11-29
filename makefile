# build an executable named myprog from myp
CC=gcc
CFLAGS=-I.

make: make ecg.o
	 $(CC) -Wall ecg.c ecg_test.c alarm.c radio.c -o ecg
clean: $(RM) ecg
