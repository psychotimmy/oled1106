/******************************************************************************/
/*                                                                            */
/* Header file for                                                            */
/* SH1106 132x64 (128x64) pixel OLED display library for I2C bus.             */
/* Writen for a Raspberry Pi 3B+ using the Raspbian Buster operating system.  */
/* Prerequisite: PIGPIOD must be installed and running.                       */
/*                                                                            */
/* (c) Tim Holyoake, 2nd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pigpiod_if2.h>

/* SH1106 I2C device address */

#define SH1106ADDR      0x3C    // I2C address of OLED. Some use 0x3D instead.

/* Declare SH1106 library functions as externals */

extern void olederror_fprintf(int errnum);
extern int oledflushfb(int pi, int fd);
extern int oledstr(int pi, int fd, char *writebuf, uint8_t page, uint8_t fontnum, uint8_t fbwrite);
extern int oledclear(int pi, int fd, uint8_t fbwrite);
extern int oledinit(int pi, int fd);
extern int oledoff(int pi, int fd);
extern int oledon(int pi, int fd);
extern int oledrv(int pi, int fd);
extern int olednv(int pi, int fd);
extern int oledsetpage(int pi, int fd, int pageno);
extern int oledresetcol(int pi, int fd); 
extern int oledresetline(int pi, int fd);
extern int oledhorizline(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t xlen, uint8_t mode, uint8_t fbwrite);
extern int oledvertline(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t ylen, uint8_t mode, uint8_t fbwrite);
extern int oledrectangle(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t xlen, uint8_t ylen, uint8_t mode, uint8_t fbwrite);
extern int oledfillrect(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t xlen, uint8_t ylen, uint8_t mode, uint8_t fbwrite);
extern int oledcircle(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t r, uint8_t mode, uint8_t fbwrite);
extern int oledfillcircle(int pi, int fd, uint8_t startx, uint8_t starty, uint8_t r, uint8_t mode, uint8_t fbwrite);
extern int oledsetpixel(int pi, int fd, uint8_t x, uint8_t y, uint8_t mode, uint8_t fbwrite);
