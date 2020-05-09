#
# Makefile for the SH1106 oled library and samples.
# (C) Tim Holyoake, 8th May 2020.
# Typing 'make' will create the library and sample programs
# Typing 'make oled1106test' will create a skeleton executable.
# Typing 'make oled1106life' will create a Conway's life game.
#

CC = gcc
RM = rm
CFLAGS = -Wall -lpigpiod_if2

default: oled1106test oled1106life oled1106.a

oled1106.a: oled1106global.o oled1106.o 
	ar -crs oled1106.a oled1106global.o oled1106.o

oled1106test: oled1106test.o oled1106.a
	$(CC) $(CFLAGS) -o oled1106test oled1106test.o oled1106.a
	strip oled1106test

oled1106life: oled1106life.o oled1106.a
	$(CC) $(CFLAGS) -o oled1106life oled1106life.o oled1106.a
	strip oled1106life

oled1106global.o:  oled1106global.c oled1106.h 
	$(CC) $(CFLAGS) -c oled1106global.c

oled1106.o:  oled1106.c oled1106.h
	$(CC) $(CFLAGS) -c oled1106.c

oled1106test.o:  oled1106test.c oled1106.h
	$(CC) $(CFLAGS) -c oled1106test.c

oled1106life.o:  oled1106life.c oled1106.h
	$(CC) $(CFLAGS) -c oled1106life.c

clean: 
	$(RM) *.a *.o oled1106test oled1106life
