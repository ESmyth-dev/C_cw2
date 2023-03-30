/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */ 
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */  
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */


#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


// APP constants   ---------------------------------

// Wiring (see call to lcdInit in main, using BCM numbering)
// NB: this needs to match the wiring as defined in master-mind.c

#define STRB_PIN 24
#define RS_PIN   25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

// -----------------------------------------------------------------------------
// includes 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// -----------------------------------------------------------------------------
// prototypes

int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
void digitalWrite (uint32_t *gpio, int pin, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
}

// adapted from setPinMode
void pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {
  // This is in C, needs to converted to inline Assembler!!!
  int fSel =  pin/10;
  int shift = (pin%10)*3;
  if(mode==HIGH){
    *(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift)) | (1 << shift); // set bits to one = output
  } else{
    *(gpio + fSel) = (*(gpio + fSel) & (7 << shift)); // set bits to zero = input
  }
}

void writeLED(uint32_t *gpio, int led, int value) {
  // This is in C, needs to converted to inline Assembler!!!
  if ((led & 0xFFFFFFC0) == 0) // sanity check
  {
    int clrOff, setOff;
    if (led < 32)
    {
      clrOff = 10; // GPCLR, register for clearing a pin value
      setOff = 7; // GPSET, register for setting a pin value
    }
    else
    {
      clrOff = 11;
      setOff = 8;
    }

    if (value == LOW) 
      *(gpio + clrOff) = 1 << (led & 31) ;
    else
      *(gpio + setOff) = 1 << (led & 31) ;
  } 
  else 
  { 
    fprintf(stderr, "brother out here not using on-board pins frfr 💀💀💀\n"); 
    exit(1); 
  }
}

int readButton(uint32_t *gpio, int button) {
  // This is in C, needs to converted to inline Assembler!!!
  if(button<32){
    if ((*(gpio + 13 /* GPLEV0 */) & (1 << (button & 31))) != 0)
    return HIGH ;
  else
    return LOW ;
  } else{
  if ((*(gpio + 14 /* GPLEV0 */) & (1 << (button & 31))) != 0)
    return HIGH ;
  else
    return LOW ;
  }
}

void waitForButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, just C no Assembler; you can use readButton ***  */
}
