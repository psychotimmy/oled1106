/******************************************************************************/
/*                                                                            */
/* Conway's game of life program for                                          */
/* SH1106 132x64 (128x64) pixel OLED display library for I2C bus.             */
/* Tested on a Raspberry Pi 3B+ using the Raspbian Buster operating system.   */
/*                                                                            */
/* Prerequisite: PIGPIOD must be installed and running.                       */
/*                                                                            */
/* (c) Tim Holyoake, 9th May 2020.                                            */
/*                                                                            */
/******************************************************************************/
#include <time.h>
#include "oled1106.h"

#define GENMAX 		20000
#define STABLEMAX 	50

static int maxliving, minliving, genstable;

int nextgen(int pi, int fd, void *bd, int columns, int rows)
{
	int living,x,y,x1,y1,tc,tr;
	uint8_t ncells; 
	uint8_t newboard[columns][rows];
	uint8_t (*board)[rows]= bd;

	living=0;

	// Work out what the next generation should look like from the current
	for (x=0; x<columns; x++) {
		for (y=0; y<rows; y++) {
			ncells=0;
			for(x1=x-1; x1<=x+1; x1++) {
				for(y1=y-1; y1<=y+1; y1++) {
                                        tc=(x1+columns)%columns;
					tr=(y1+rows)%rows;
					if (board[tc][tr] == PIXON) ++ncells;
				}
			}
			if (board[x][y] == PIXON) --ncells; // Don't count the cell we're examining if it's alive
			if (ncells == 3 || ((ncells == 2) && (board[x][y] == PIXON))) {
				newboard[x][y] = PIXON;
				++living;
			}
			else {
				newboard[x][y] = PIXOFF;
			}
			
			(void) oledsetpixel(pi,fd,x+ORIGIN,y+ORIGIN,newboard[x][y],FBONLY);
		}
	}

	// Overwite the old board with the new board
	for (x=0; x<columns; x++) 
		for (y=0; y<rows; y++) 
			board[x][y]=newboard[x][y];

	return(living);
}

void life(int pi, int fd, int columns, int rows)
{
	int lasttest,living,gens;
	time_t t;
	uint8_t x,y;
	uint8_t board[columns][rows];

	while (1) {

		gens=0;
		lasttest=0;
	
		// Initialize the board with a random pattern - one in twelve pixels on.
		srand((unsigned) time(&t)); 
		for (x=0; x<columns; x++) {
			for (y=0; y<rows; y++) {
				board[x][y]=rand() < RAND_MAX/12 ? PIXON : PIXOFF;
				(void) oledsetpixel(pi,fd,x+ORIGIN,y+ORIGIN,board[x][y],FBONLY);
			}
		}

		// Set living cells to 1 (obviously > 1 in all but bizarrely random circumstances
		living=1;

		// Reset genstable and maxliving variables to zero (ignore the starting cell colony count)
		// and minliving to COLUMNS*ROWS
		genstable=0;
		maxliving=0;
		minliving=COLUMNS*ROWS;

                // Get the next generation provided some cells are still living or 1000 generations have passed
		// and the pattern is not stable or repeating (ish - this algortihm is not exact)

		while ((living > 0) && (gens < GENMAX) && (genstable < STABLEMAX)) {	

			// Flush the framebuffer and compute the next generation
			(void) oledflushfb(pi,fd);
			lasttest=living;
			living=nextgen(pi,fd,board,columns,rows);
			++gens;

			// Update the static variables
			if(living > maxliving) maxliving=living;
			if(living < minliving) minliving=living;
			if (living == lasttest) { 
				++genstable;
			}
			else {
				genstable=0;
			}
		}

		printf("Last simulation ended after %d generations with %d living cells\n",gens,living);
		printf("Maximum living cells was %d, minimum was %d\n",maxliving,minliving);
                fflush(stdout);

		// Pause for 10 seconds before starting again
		sleep(10);
	}

	return;
}

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
		// Play the game on the full resolution of the display
		life(ipi,fdoled,COLUMNS,ROWS);

	}

        /* Clean up and exit */

        i2c_close(ipi,fdoled);
        pigpio_stop(ipi);

	return(i);
}
