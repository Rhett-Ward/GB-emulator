#ifndef GBCPU_H
#define GBCPU_H

/*
Start date: 07/24/2025
Last updated: 07/24/2025
Name: Rhett Ward
Update log:
    *07/24/2025
        made this header in order to follow proper file organization and use for c code
        added ifndef to properly implement the cooperation of MMU and CPU (ifndef prevents infinite looping)
*/

//Includes needed
#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16 
#include "MMU.h" // memory manipulation unit interface

//FLAGS
#define Z_FLAG 0x80 // bit 7 (zero flag)
#define N_FLAG 0x40 // bit 6 (subtraction flag)
#define H_FLAG 0x20 // bit 5 (Half Carry Flag)
#define C_FLAG 0x10 // bit 4 (Carry Flag)
/*
 * will take a minute here to explain the gameboy flag system as i understand it
 
 f is a uint8_t register that holds flags based on the first 4 bits of the 8 bits

 hexadecimal notation "0x##" is a undefined size notation that is translated to 8 bits by breaking the two numbers after "0x" into their 4 bit equivalents

 so hex notation is "0x" followed by upper 4 bit and lower 4 bit meaning "0x80" is 1000 + 0000 is the 8 bit set : 1000 0000
 */


/**
 * @brief Structure that holds the GB cpu registers
 */
struct GB_Registers{

    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint8_t f; // flags register, used to indicate information about the last operation

    uint8_t ime; // Interrupt flag

    //my 16 bit registers (uint16_t is specifcally a 16 bit int)
    uint16_t pc;
    uint16_t sp;

    //Clock registers
    int m;
    int t;
};

/**
 * @brief Structure for the GB CPU clock
 */
struct GB_Clock{
 int m;
 int t;
};

/**
 * @brief Structure that acts as the GB CPU
 */
struct GB_CPU {
    struct GB_Registers _r;
    struct GB_Clock _c;
    struct MMU mmu;
};

#endif