/******************************************************************************/
/*                                                                            */
/* Skeleton test progam for the                                               */
/* SH1106 132x64 (128x64) pixel OLED display library for I2C bus.             */
/* Tested on a Raspberry Pi 3B+ using the Raspbian Buster operating system.   */
/*                                                                            */
/* Prerequisite: PIGPIOD must be installed and running.                       */
/*                                                                            */
/* (c) Tim Holyoake, 23rd April 2020.                                         */
/*                                                                            */
/******************************************************************************/
#include "oled1106.h"

/* Testing loop */

int main() {
        int ipi,fdoled,i; 			

        ipi=pigpio_start(NULL,NULL);	// Initialise connection to pigpiod */ 
        if (ipi < 0) {
		fprintf(stderr,"Failed to connect to pigpiod - error %d\n",ipi);
                exit(1);
        }

        fdoled=i2c_open(ipi,1,SH1106ADDR,0); // Get handle to 128x64 OLED display
        if (fdoled < 0) {
		fprintf(stderr,"Failed to initialize OLED - error %d\n",fdoled);
                exit(1);
        }

	// Initialize the oled display

        if (oledinit(ipi,fdoled) == 0) {

		// Clear the display
		i=oledclear(ipi,fdoled,FBANDDISPLAY);
		// Write text on the second line of the display
		i=oledstr(ipi,fdoled,"  Tim Holyoake  ",7,0,FBANDDISPLAY);
		// Write text on the next line of the display
                i=oledstr(ipi,fdoled,"  8th May 2020  ",6,0,FBANDDISPLAY);
		// Draw a border all the way around the display
		i=oledrectangle(ipi,fdoled,1,1,127,63,PIXON,FBANDDISPLAY);
		// Draw a filled circle
		i=oledfillcircle(ipi,fdoled,64,25,10,PIXINV,FBANDDISPLAY);

		sleep(3); 	// Wait

		// Clear the display
		i=oledclear(ipi,fdoled,FBANDDISPLAY);

		// Repeat in framebuffer only mode

		// Write text on the second line of the display
		i=oledstr(ipi,fdoled,"  Tim Holyoake  ",7,0,FBONLY);
		// Write text on the next line of the display
                i=oledstr(ipi,fdoled,"  8th May 2020  ",6,0,FBONLY);
		// Draw a border all the way around the display
		i=oledrectangle(ipi,fdoled,1,1,127,63,PIXON,FBONLY);
		// Draw a filled circle
		i=oledfillcircle(ipi,fdoled,64,25,10,PIXINV,FBONLY);
		// Flush the framebuffer to the display
		i=oledflushfb(ipi,fdoled);

	}

        /* Clean up and exit */

        i2c_close(ipi,fdoled);
        pigpio_stop(ipi);

	return(i);
}
