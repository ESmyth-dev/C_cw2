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
extern int timerActive;

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
void digitalWrite (uint32_t *gpio, int pin, int value) {
  asm(
    "MOV R0, #1\n"
    "MOV R1, #31\n"
    "AND R1, %[pin]\n"
    "LSL R0, R0, R1\n" //shift #1 (pin & 31) to the left
    "CMP %[pin], #32\n"
    "BLT less\n"
    "MOV R1, #11\n" //GPCLR
    "MOV R2, #8\n"  //GPSET
    "B continue\n"

  "less:\n"
    "MOV R1, #10\n"
    "MOV R2, #7\n"
    "B continue\n"

  "continue:\n"
    "CMP %[val], #0\n"
    "BEQ low\n"
    "LDR R3, [%[gpio], R2]\n" // r3 = *(gpio + GPSET)
    "B end\n"

  "low:\n"
    "LDR R3, [%[gpio], R1]\n" // r3 = *(gpio + GPCLR)
    "B end\n"

  "end:\n"
    "MOV R3, R0\n"

    : // no output
    : [gpio] "r" (gpio)
      , [pin] "r" (pin)
      , [val] "r" (value)
    : "r0", "r1", "r2", "cc" );

  /* // Same code in C
  if ((pin & 0xFFFFFFC0) == 0) // sanity check
  {
    int clrOff, setOff;
    if (pin < 32)
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
      *(gpio + clrOff) = 1 << (pin & 31) ;
    else
      *(gpio + setOff) = 1 << (pin & 31) ;
  } 
  else 
  { 
    fprintf(stderr, "brother out here not using on-board pins frfr 💀💀💀\n"); 
    exit(1); 
  } */
}

// adapted from setPinMode
void pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {
  asm(
    "UDIV R0, %[pin], #10\n"  // r0 = pin/10
    "MUL R1, R0, #10\n"      
    "SUB R1, %[pin], R1\n"
    "MUL R1, R1, #3\n"        // r1 = (pin%10) * 3
    "MOV R2, #7\n"            // r2 = 0b111
    "LDR R3, [%[gpio], R0]\n" // r3 = *(gpio + r0)
    "LSL R2, R2, R1\n"        // r2 << r1
    "BIC R2, R2\n"            // r2 = ~(r2)
    "AND R3, R2\n"            // r3 & r2
    "CMP %[mode], #1\n"
    "BEQ output\n" // if mode = OUTPUT, go to "input" branch, else just leave the bits cleared
    "B end\n"

  "output:\n"
    "MOV R2, #1\n"           // r2 = 0b001
    "LSL R2, R2, R1\n"       // r2 << r1
    "OR R3, R2\n"            // r3 | r2
    
  "end:\n"
    : // no output
    : [gpio] "r" (gpio)
      , [pin] "r" (pin)
      , [mode] "r" (mode)
    : "r0", "r1", "r2", "r3", "cc" );
      

  /* // Same code in C 
  int fSel =  pin/10;
  int shift = (pin%10)*3;
  if(mode==OUTPUT){
    *(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift)) | (1 << shift); // set bits to one = output
  } else{
    *(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift)); // set bits to zero = input
  } */
}

void writeLED(uint32_t *gpio, int led, int value) {
  // This is in C, needs to converted to inline Assembler!!!
  digitalWrite(gpio,led,value);
}

int readButton(uint32_t *gpio, int button) {
  int result;
  
  asm(
    "CMP %[button], #32\n"
    "BLT less\n"
    "MOV R0, #14\n" //GPLEV0
    "B continue\n"
  "less:\n"
    "MOV R0, #13\n"
  
  "continue:\n"
    "LDR R1, [%[gpio], R0]\n"
    "MOV R2, #1\n"
    "MOV R3, #31\n"
    "AND R3, %[button]\n"
    "LSL R2, R2, R3\n" //shift #1 (button & 31) to the left
    "AND R2, R1\n"
    "CMP R2, #0\n"
    "BEQ low\n"
    "MOV %[result], 1\n"
    "B end:\n"

  "low:\n"
    "MOV %[result], 0\n"
  
  "end:\n"
    : [result] "=r" (result)
    : [gpio] "r" (gpio)
      , [button] "r" (button)
    : "r0", "r1", "r2", "cc" );

  return result;

  /* Same code in C
  if(button<32){
    if ((*(gpio + 13) & (1 << (button & 31))) != 0)
    return HIGH;
  else
    return LOW;
  } else{
  if ((*(gpio + 14) & (1 << (button & 31))) != 0)
    return HIGH;
  else
    return LOW;
  } */
}

void waitForButton(uint32_t *gpio, int button) {
  int timerChange = timerActive;
  while(TRUE){
    if(timerChange!=timerActive)
      break;
    if(readButton(gpio,button)==LOW)
      break;
    usleep(50000);
  }
  while(TRUE){
    if(timerChange!=timerActive)
      break;
    if(readButton(gpio,button)==HIGH)
      break;
    usleep(50000);
  }
}
