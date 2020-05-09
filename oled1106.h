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

/* SH1106 general definitions */

#define PIXOFF		0	// Set pixel off
#define PIXON		1	// Set pixel on
#define PIXINV		2	// Invert pixel (ON becomes OFF, OFF becomes ON)
#define FBONLY          1       // Write to the framebufffer only
#define FBANDDISPLAY    2       // Write to the framebuffer and display simultaneously.
#define COLOFFSET       0x02	// As the addressable RAM is 132x64, but the
                                // display is 128x64, need a column offset.
#define COLUMNS         128     // Display has 128 columns of visible pixels.
#define ROWS            64      // Display has 64 rows of visible pixels.
#define PAGES           8       // The top line of the diplay is on page 8.
#define ROWSPERPAGE     8       // 8 rows of pixels per page = 64 rows per display.
#define CHARSPERPAGE    16      // The number of 8x8 character cells per page.
#define STDCHARWIDTH    8       // The width of a 8x8 character cell.
#define ORIGIN    	1	// Defines the origin point for the library. 
				// Default is 1 (i.e. first co-ordinate is (1,1)
				// to make it friendlier to use the library
				// with FORTRAN. It may work with 0 (if you really
				// want the bottom left co-ordinate to be (0,0)
				// but this is untested.

/* SH1106 library error codes */

#define PAGETOOLOW	-1000   // Page specified as 0 or lower
#define PAGETOOHIGH     -1001   // Page specified as 9 or higher
#define BADPIXELCMD     -1002   // Command not PIXON, PIXOFF or PIXINV
#define COLOUTOFRANGE   -1003   // Column is < ORIGIN or > ORIGIN+127
#define ROWOUTOFRANGE   -1004   // Row is < ORIGIN or > ORIGIN+63
#define NEGORZERORADIUS -1005   // Tried to draw a circle with negative or zero radius
#define INVALIDFBCODE   -1006   // Framebuffer write code is invalid

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

/* Declare SH1106 global variables as externals */

extern char oled1106fb[PAGES][COLUMNS];		// In-memory franebuffer
extern char oledf8x8[96][STDCHARWIDTH];        	// Charcter font definitions
