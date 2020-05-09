/******************************************************************************/
/*                                                                            */
/* SH1106 132x64 (128x64) pixel OLED display library for I2C bus.             */
/* Writen for a Raspberry Pi 3B+ using the Raspbian Buster operating system.  */
/* Prerequisite: PIGPIOD must be installed and running.                       */
/*                                                                            */
/* (c) Tim Holyoake, 23rd April 2020.                                         */
/*                                                                            */
/******************************************************************************/
#include "oled1106.h"

/* SH1106 library functions */

void olederror_fprintf(int errnum) {
/******************************************************************************/
/*                                                                            */
/* Print error code description returned from sh1106 function to stderr.      */
/* Error codes start at -1000 and descend to -1004.                           */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
        char errcode[7][80]={"Page number too low (less than 1) specified",
                             "Page number too high (greater than 8) specified",
                             "Invalid pixel mode - not PIXON, PIXOFF or PIXINV",
                             "Invalid x co-ordinate specified",
                             "Invalid y co-ordinate specified",
			     "Negative or zero radius for circle specified",
			     "Invalid framebuffer type specified"} ;

        if ((errnum > PAGETOOLOW) || (errnum < INVALIDFBCODE)) {
		fprintf(stderr,"Unknown SH1106 error number(%d)\n",errnum);
        }
        else {
		fprintf(stderr,"%s (%d)\n",errcode[(-1*errnum)-1000],errnum);
        }
        return;
}

int oledflushfb(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Flush the current framebuffer to the oled display.                         */
/*                                                                            */
/* (c) Tim Holyoake 9th May 2020.                                             */
/*                                                                            */
/******************************************************************************/
	int i, colcount, pgcount;
	char buf[129];

        for (pgcount=0; pgcount<PAGES; pgcount++) { 	// Loop through pages 0xB0 to 0xB7
        	buf[0]=0x00;
                buf[1]=COLOFFSET;
                buf[2]=0x10;
        	buf[3]=0xB0+pgcount;
        	i = i2c_write_device(pi,fd,buf,4);      		// Set page to be flushed
		if (i != 0) return(i);					// Error in pigpiod

		buf[0]=0x40;
		for (colcount=0; colcount<COLUMNS; colcount++) {
			buf[colcount+1]=oled1106fb[pgcount][colcount];
		}
        	i = i2c_write_device(pi,fd,buf,129);     		// Flush page to OLED
		if (i !=0) return(i);                                   // Error in pigpiod
	}

	return(0);
}

int oledstr(int pi, int fd, char *writebuf, uint8_t page, 
             uint8_t fontnum, uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Write a string of up to CHARSPERPAGE (16) characters at the start of the   */
/* specified page of the display. Page 8 = top page; page 1 = bottom page.    */
/* Currently designed to work for the printing ASCII characters 32-127 only.  */
/* Fontnum is for future development - can be used to specify an alternative  */
/* font style for example.                                                    */
/*                                                                            */
/* (c) Tim Holyoake, 3rd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int i, len;
        char buf[129];
        char transbuf[16];

     	/* Error handling - check page specified is in the range 1 - 8 */
	/* and that a valid framebuffer option has been specified */

        if (page < ORIGIN) {
		olederror_fprintf(PAGETOOLOW);
		return (PAGETOOLOW);
	} else if (page > PAGES) {
		olederror_fprintf(PAGETOOHIGH);
		return (PAGETOOHIGH);
	}

        if ((fbwrite != FBONLY) && (fbwrite !=FBANDDISPLAY)) {
		olederror_fprintf(INVALIDFBCODE);
		return (INVALIDFBCODE);
	}

	if (fbwrite == FBANDDISPLAY) {
        	buf[0]=0x00;                                // Set up the page to be written
        	buf[1]=COLOFFSET;                           // if write to display selected
        	buf[2]=0x10;
        	buf[3]=0xB0+(page-ORIGIN);
       		i = i2c_write_device(pi,fd,buf,4);                                 
	}

        /* Buffer is truncated to the page length if it is longer than 16 characters */

	len=strlen(writebuf);
	if (len > CHARSPERPAGE) len=CHARSPERPAGE;
        strncpy(transbuf,writebuf,len);

        buf[0]=0x40;				    
        for (i=0; i<len; i++) {                     
		if (((uint8_t)transbuf[i]<32) || ((uint8_t)transbuf[i]>127)) transbuf[i]=32;
        	buf[(i*8)+1]=oledf8x8[transbuf[i]-32][7];     
        	buf[(i*8)+2]=oledf8x8[transbuf[i]-32][6];     
        	buf[(i*8)+3]=oledf8x8[transbuf[i]-32][5];
    		buf[(i*8)+4]=oledf8x8[transbuf[i]-32][4];
        	buf[(i*8)+5]=oledf8x8[transbuf[i]-32][3];
        	buf[(i*8)+6]=oledf8x8[transbuf[i]-32][2];
        	buf[(i*8)+7]=oledf8x8[transbuf[i]-32][1];
        	buf[(i*8)+8]=oledf8x8[transbuf[i]-32][0];
	}
   
        if (fbwrite == FBANDDISPLAY) {
       		i = i2c_write_device(pi,fd,buf,1+(len*8));        // Write contents of buf to oled 
	}							  // if write to display selected

 	// Write the page to the framebuffer 
	for (i=1; i<1+(len*8); i++)
		oled1106fb[page-ORIGIN][i]=buf[i]; 

        return(0);
}

int oledclear(int pi, int fd, uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Clear all bits visible on the 128x64 display and/or memory framebuffer.    */
/* i.e. a blank screen in normal video mode. For all of the pages (0xB0-0xB7) */
/* 128 bytes of 0x00 from the column offestis sent to blamk them.             */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	int i,count,col;
        char buf[4];
        char blankpage[129]=   {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        for (count=0; count<PAGES; count++) {            // Loop through pages 0xB0 to 0xB7
        	buf[0]=0x00;
                buf[1]=COLOFFSET;
                buf[2]=0x10;
        	buf[3]=0xB0+count;
		if (fbwrite == FBANDDISPLAY) {			    // Blank display on request
        		i = i2c_write_device(pi,fd,buf,4);          // Set page to be blanked
			if (i !=0) return(i);			    // pigpiod error
        		i = i2c_write_device(pi,fd,blankpage,129);  // Write blank page to OLED
			if (i !=0) return(i);			    // pigpiod error
		}
                for (col=0; col<COLUMNS; col++)       		    // Clear the framebuffer
           		oled1106fb[count][col]=0x00;
	}

        return(0);
}

int oledinit(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Initialize the SH1106. See the product data sheet for detailes and other   */
/* options (it can be found on many places on the internet!)                  */
/*                                                                            */
/* (c) Tim Holyoake, 26th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
        int i;
        char buf[30];

        buf[0] =0x00;			// Set the SH1106 to receive commands.
        buf[1] =0xAE;			// Turn the OLED display off.
        buf[2] =0x81;                   // Set the display contrast ...
	buf[3] =0x80;                   // ... to the default (middle) value 0x80.
	buf[4] =0xA1;			// Set segment re-map. (A0 default, A1 reversed).
	buf[5] =0xA6;			// Set display to normal video (0xA7 is reverse video).
        buf[6] =0xA8;			// Set multiplex display ratio ...
        buf[7] =0x3F;                   // ... to 3F - i.e. use all 64 lines of the display.
        buf[8] =0xAD;                   // Set the DC-DC converter ...
        buf[9] =0x8B;                   // ... to ON (built-in)
        buf[10]=0x30;                   // Set the pump charger voltage to 6.4volts
        buf[11]=0xC0;                   // Set common output scan direction (C0 default, C8 reversed).
        buf[12]=0xD3;			// Set display offset mode ...
        buf[13]=0x00;                   // ... to 0x00 (i.e. no offest).
        buf[14]=0xD5;                   // Divide ratio / osciallator frequency is set to ...
        buf[15]=0x80;                   // ... 0x80
        buf[16]=0xD9;                   // Set pre-charge/discharge period to ...
        buf[17]=0x1F;                   // ... 0x1F = 0001 1111 binary (1 DCLK discharge period, 15 DCLK pre-charge period).
        buf[18]=0xDA;			// Set common pads configuration ...
        buf[19]=0x12;                   // ... to 0x12 (0x02 = sequential, 0x12 = alternative)
        buf[20]=0xDB;                   // Set VCOM deselect level to ...
        buf[21]=0x40;                   // ... 0x40 = 1volt (any value between 0x40 and 0xFF has the same effect).

        i=i2c_write_device(pi,fd,buf,22); // Ignore any pigpiod errors for the moment ...

        oledclear(pi,fd,FBANDDISPLAY);  // Clear the display RAM
        buf[0] =0x00;                   // Set the SH1106 to recieve commands.
	buf[1] =COLOFFSET;              // Set the lower column address of the display to 0x02.
        buf[2] =0x10;			// Set the higher column address of the display to 0x10.
        buf[3] =0x40;			// Set the display start line to 0x40.
        buf[4] =0xAF;                   // Turn the OLED display on now initialization is complete.

        i=i2c_write_device(pi,fd,buf,5);

        return(i);
}
	
int oledoff(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Turn off the SH1106 OLED display.                                          */
/* Requires the command code 0x00 followed by 0xAE sending to the I2C address */
/* of the device.                                                             */
/*                                                                            */
/* (c) Tim Holyoake, 24th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[2] = {0x00, 0xAE};

        return(i2c_write_device(pi,fd,buf,2));
}

int oledon(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Turn on the SH1106 OLED display.                                           */
/* Requires the command code 0x00 followed by 0xAF sending to the I2C address */
/* of the device.                                                             */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[2] = {0x00, 0xAF};

        return(i2c_write_device(pi,fd,buf,2));
}

int oledrv(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Put the SH1106 OLED display into reverse video mode.                       */
/* Requires the command code 0x00 followed by 0xA7 sending to the I2C address */
/* of the device.                                                             */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[2] = {0x00, 0xA7};

        return(i2c_write_device(pi,fd,buf,2));
}

int olednv(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Put the SH1106 OLED display into normal video mode.                        */
/* Requires the command code 0x00 followed by 0xA6 sending to the I2C address */
/* of the device.                                                             */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[2] = {0x00, 0xA6};

        return(i2c_write_device(pi,fd,buf,2));
}

int oledsetpage(int pi, int fd, int pageno) {
/******************************************************************************/
/*                                                                            */
/* Set the page number on the display for output.                             */
/* Requires the command code 0x00 followed by 0xB0-0xB7 depending on the page */
/* required. This function accepts values in the range 1 - 8. Page 1 is the   */
/* bottom line of the display, page 8 is the top line.                        */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	int i;
	char buf[2];

	/* Error handlling - check pageno is in range 1 - 8 */

        if (pageno < 1) {
                olederror_fprintf(PAGETOOLOW);
		return (PAGETOOLOW);
	}

        else if (pageno > 8) {
                olederror_fprintf(PAGETOOHIGH);
		return (PAGETOOHIGH);
	}

	/* Set the active page */

        else {
		buf[0]=0x00;
		buf[1]=0xB0+pageno-1;
        	i = i2c_write_device(pi,fd,buf,2);
        	return(i);
	}

	return(0);
}

int oledresetcol(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Resets the column number on the display for output.                        */
/* Requires the command code 0x00 followed by 0x02 (COLOFFSET) 0x10           */
/*                                                                            */
/* (c) Tim Holyoake, 25th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[3] = {0x00, COLOFFSET, 0x10};

       	return(i2c_write_device(pi,fd,buf,3));
}

int oledresetline(int pi, int fd) {
/******************************************************************************/
/*                                                                            */
/* Resets the line number on the display for output.                          */
/* Requires the command code 0x00 followed by 0x40                            */
/*                                                                            */
/* (c) Tim Holyoake, 26th April 2020.                                         */
/*                                                                            */
/******************************************************************************/
	char buf[2] = {0x00, 0x40};

       	return(i2c_write_device(pi,fd,buf,2));
}

int oledhorizline(int pi, int fd, uint8_t startx, 
                                  uint8_t starty, 
                                  uint8_t xlen, 
                                  uint8_t mode,
				  uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a horizonal line on the display from startx,starty xlen pixels long. */
/* Mode is any of PIXON, PIXOFF or PIXINV.                                    */
/* Current version calls oledsetpixel xlen times and so is sub-optimal        */
/*                                                                            */
/* (c) Tim Holyoake, 2nd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int i;

	/* Error handling - check startx, startx+xlen, starty and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx+xlen > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

	/* Draw the line */

	for (i=startx; i<=startx+xlen; i++) 
		oledsetpixel(pi,fd,i,starty,mode,FBONLY);

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}

int oledvertline(int pi, int fd, uint8_t startx, 
                                 uint8_t starty, 
                                 uint8_t ylen, 
                                 uint8_t mode,
				 uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a vertical line on the display from startx,starty ylen pixels long.  */
/* Mode is any of PIXON, PIXOFF or PIXINV.                                    */
/* Current version calls oledsetpixel ylen times and so is sub-optimal        */
/*                                                                            */
/* (c) Tim Holyoake, 2nd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int i;

	/* Error handling - check startx, starty, starty+ylen and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty+ylen > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

	/* Draw the line */

	for (i=starty; i<=starty+ylen; i++)
		oledsetpixel(pi,fd,startx,i,mode,FBONLY);

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}


int oledrectangle(int pi, int fd, uint8_t startx, 
                                  uint8_t starty, 
                                  uint8_t xlen, 
                                  uint8_t ylen, 
                                  uint8_t mode,
				  uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a rectangle on the display from startx,starty xlen pixels wide and   */
/* ylen pixels high. Mode is any of PIXON, PIXOFF or PIXINV.                  */
/* Current version calls oledsetpixel xlen*2 + ylen*2 times and so is sub-    */
/* optimal.                                                                   */
/*                                                                            */
/* (c) Tim Holyoake, 2nd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int i;

	/* Error handling - check startx, startx+xlen, starty, starty+ylen and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx+xlen > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty+ylen > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

	/* Draw the four sides of the rectangle bottom left -> bottom right -> top right -> top left -> bottom left */

	for (i=startx; i<startx+xlen; i++)
		oledsetpixel(pi,fd,i,starty,mode,FBONLY);
        for (i=starty;i<starty+ylen; i++)
		oledsetpixel(pi,fd,startx+xlen,i,mode,FBONLY);
	for (i=startx+xlen; i>startx; i--)
		oledsetpixel(pi,fd,i,starty+ylen,mode,FBONLY);
	for (i=starty+ylen; i>starty; i--)
		oledsetpixel(pi,fd,startx,i,mode,FBONLY);

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}

int oledfillrect(int pi, int fd, uint8_t startx, 
                                 uint8_t starty, 
                                 uint8_t xlen, 
                                 uint8_t ylen, 
                                 uint8_t mode,
				 uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a filled rectangle on the display from startx,starty xlen pixels     */
/* wide and ylen pixels high. Mode is any of PIXON, PIXOFF or PIXINV.         */
/* Current version calls oledsetpixel xlen*ylen times and so is sub-optimal.  */
/*                                                                            */
/* (c) Tim Holyoake, 2nd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int x,y;

	/* Error handling - check startx, startx+xlen, starty, starty+ylen and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx+xlen > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty+ylen > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

	/* Draw the filled rectangle from the bottom line upwards */

	for (y=starty; y<=starty+ylen; y++) {
		for (x=startx; x<=startx+xlen; x++)
			oledsetpixel(pi,fd,x,y,mode,FBONLY);
	}

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}

int oledcircle(int pi, int fd, uint8_t startx, 
                               uint8_t starty, 
                               uint8_t r,
                               uint8_t mode,
			       uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a circle on the display with centre (startx,starty) with radius r.   */
/* If the circle has a centre off the display errors are returned.            */
/* If the circle can only be partly drawn because the radius exceeds the      */
/* display area at certain points, then the arc (or arcs) that can be drawn   */
/* are displayed.                                                             */
/* Mode is any of PIXON, PIXOFF or PIXINV.                                    */
/* Current version calls oledsetpixel many times and so is sub-optimal.       */
/*                                                                            */
/* (c) Tim Holyoake, 3rd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
        int x,y,c,c1;

	/* Error handling - check startx, starty, r and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

        if (r < 1) {
		olederror_fprintf(NEGORZERORADIUS);
		return(NEGORZERORADIUS);
	}

	/* Draw the circle */

        c=r*r;
        c1=(r+1)*(r+1);

	for(y=-r; y<=r; y++) {
		for(x=-r; x<=r; x++)
		{
			if(((x*x)+(y*y)) < c)
			{
				// Nothing to draw - the pixel is not needed to be set
			}
			else if(((x*x)+(y*y)) < c1)
			{
				if ((startx+x < ORIGIN) || (startx+x > COLUMNS-ORIGIN) ||
                                    (starty+y < ORIGIN) || (starty+y > ROWS-ORIGIN)) {
					// Nothing to draw - the pixel is out of range
				} 
				else {
					// A new pixel needs to be set
					oledsetpixel(pi,fd,startx+x,starty+y,mode,FBONLY);
				}
			}
		}
	}

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}

int oledfillcircle(int pi, int fd, uint8_t startx, 
                               uint8_t starty, 
                               uint8_t r,
                               uint8_t mode,
			       uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Draws a filled circle on the display with centre (startx,starty) radius r. */
/* If the circle has a centre off the display errors are returned.            */
/* If the circle can only be partly drawn because the radius exceeds the      */
/* display area at certain points, then a filled circle is drawn, cropped to  */
/* the edges of the display.                                                  */
/* Mode is any of PIXON, PIXOFF or PIXINV.                                    */
/* Current version calls oledsetpixel many times and so is sub-optimal.       */
/*                                                                            */
/* (c) Tim Holyoake, 3rd May 2020.                                            */
/*                                                                            */
/******************************************************************************/
        int x,y,c;

	/* Error handling - check startx, starty, r and mode parameters are in range */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((startx < ORIGIN) || (startx > COLUMNS)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((starty < ORIGIN) || (starty > ROWS)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

        if (r < 1) {
		olederror_fprintf(NEGORZERORADIUS);
		return(NEGORZERORADIUS);
	}

	/* Draw the filled circle */

        c=r*r;

	for(y=-r; y<=r; y++) {
		for(x=-r; x<=r; x++)
		{
			if(((x*x)+(y*y)) <= c)
			{
				if ((startx+x < ORIGIN) || (startx+x > COLUMNS-ORIGIN) ||
                                    (starty+y < ORIGIN) || (starty+y > ROWS-ORIGIN)) {
					// Nothing to draw - the pixel is out of range
				} 
				else {
					// A new pixel needs to be set
					oledsetpixel(pi,fd,startx+x,starty+y,mode,FBONLY);
				}
			}
		}
	}

	/* Flush to display if this is required */

	if (fbwrite == FBANDDISPLAY) oledflushfb(pi,fd);

	return(0);
}

int oledsetpixel(int pi, int fd, uint8_t x, uint8_t y, uint8_t mode, 
                 uint8_t fbwrite) {
/******************************************************************************/
/*                                                                            */
/* Sets a specific pixel on the display to ON or OFF without scrolling.       */
/* Requires the command code 0x00 followed by the column (x-1) and then page  */
/* (uint8_t)(y/8)-1 (There are 8 lines per page, 64 lines = 8 pages).         */
/*                                                                            */
/* If mode is PIXON:                                                          */
/* The column byte becomes the bitwise or of 0x01 if (y%8)-1 is 0, 0x02 if 1, */
/* 0x04 if 2, 0x08 if 3, 0x10 if 4, 0x20 if 5, 0x40 if 6 and 0x80 if 7.       */
/*                                                                            */
/* If mode if PIXOFF:                                                         */
/* The column byte becomes the bitwise and of the ones complement of 0x01,    */
/* 0x02, 0x04 etc.                                                            */
/*                                                                            */
/* If mode if PIXINV:                                                         */
/* The column byte becomes the bitwise xor of 0x01, 0x02, 0x04 etc.           */
/*                                                                            */
/* Pixel 1,1 is bottom left of the display, 128,64 is the top right.          */
/* if ORIGIN is defined as 1 (the source code's default in oled1106.h).       */
/*                                                                            */
/* (c) Tim Holyoake, 1st May 2020.                                            */
/*                                                                            */
/******************************************************************************/
	int i;
	uint8_t col, page;
	char buf[4];

	/* Error handling - check x,y,mode and fbwrite parameters are ok */

        if ((mode < PIXOFF) || (mode > PIXINV)) {
		olederror_fprintf(BADPIXELCMD);
		return(BADPIXELCMD);
	} 

        if ((x < ORIGIN) || (x > COLUMNS+ORIGIN-1)) {
		olederror_fprintf(COLOUTOFRANGE);
		return(COLOUTOFRANGE);
	}

	if ((y < ORIGIN) || (y > ROWS+ORIGIN-1)) {
		olederror_fprintf(ROWOUTOFRANGE);
		return(ROWOUTOFRANGE);
	}

        if ((fbwrite != FBONLY) && (fbwrite != FBANDDISPLAY)) {
		olederror_fprintf(INVALIDFBCODE);
		return(INVALIDFBCODE);
	}

	/* locate the column (0-127) and page (0-7) on the display for the pixel */

        col = x-ORIGIN;
        page = (y-ORIGIN)/ROWSPERPAGE;

	/* set the display to the correct column (with offest) and page */
	if (fbwrite == FBANDDISPLAY) {
		buf[0] = 0x00;
 		buf[1] = (col+COLOFFSET) & 0x0F;
        	buf[2] = 0x10 | (((col+COLOFFSET) & 0xF0 ) >> 4);
        	buf[3] = 0xB0+page;
       		i = i2c_write_device(pi,fd,buf,4);
		// If an error was detected in pigpiod return immediately.
        	if (i != 0) return(i);		    
	}

        // Read the framebuffer, perform the correct operation for the specific
        // pixel required of the 8 (ON, OFF or INVERT), update the framebuffer
        // then finally update the display if required.

	if (mode == PIXON) {
		buf[1]=oled1106fb[page][col]|(0x01 << (y-ORIGIN)%ROWSPERPAGE);
                oled1106fb[page][col]=buf[1];
	}
	else if (mode == PIXOFF) {
		buf[1]=oled1106fb[page][col]&(~(0x01 << (y-ORIGIN)%ROWSPERPAGE));
                oled1106fb[page][col]=buf[1];
	}
        else {  // assume mode is PIXINV
		buf[1]=oled1106fb[page][col]^(0x01 << (y-ORIGIN)%ROWSPERPAGE);
                oled1106fb[page][col]=buf[1];
	}

	if (fbwrite == FBANDDISPLAY) {
		buf[0]=0x40;
       		i = i2c_write_device(pi,fd,buf,2);	// Update the oled if requested
	}

	return(i);
}
