/*
Start date: 06/23/2025
Last updated: 06/23/2025
Name: Rhett Ward
Update log:
    *06/23/2025
        Created File
    *07/10/2025
        rewrote to be MMU.h
        added comments
*/

/* Summary of file:
Outline a structure for MMU, setup structures, comment on types
*/


#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16
#include <stdbool.h> // for access to boolean

static const uint8_t bios[256] = {};

struct MBC{
    uint16_t rombank;
    uint8_t rambank;
    bool ramon;
    bool mode;
};

struct MMU{

    uint8_t* rom; // initilaized as a pointer, memory assigned from "malloc"
    uint8_t* eram; // initilaized as a pointer, memory assigned from "malloc"
    //quickly discuss why to use malloc: we use malloc bc it prevents empty mismanaged memory. It does a good job at self managing. Can be size controlled with if statements surrounding its initilaization.

    uint8_t wram[8192]; // fixed size array since the working ram has fixed amount of memory access
    uint8_t zram[127]; //fixed size array since the zero-page ram has a fixed amount of memory access

    uint32_t romoffs;
    uint32_t ramoffs;
    //these are memory offset variables, i need to learn more about memory offset to understand their function, but we use 32bit size bc 16 is too small, we want to stick to standard sizes, and the extra space of a 32 bit is negligble performance wise
    
    bool inbios; // standard yes or no "am i in bios" or "am i not"
    uint8_t carttype; // 8 bit int that will clarify the type of cartridge thats being loaded
    uint8_t ie; // interupt enable flag
    uint8_t _IF; // interupt flag flag

    struct MBC mbc;
};

