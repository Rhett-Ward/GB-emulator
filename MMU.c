/*
Start date: 07/10/2025
Last updated: 07/10/2025
Name: Rhett Ward
Update Log:
    *07/10/2025
        added summary
        built out a memory reset function
        built out a memory load function
        created an outline for rb
    *07/21/2025
        fleshed out rb
        wrote rw
        wrote wb
        wrote ww
        comments needed
*/

/* Summary of file:
implement functionality of mmu->h
*/

#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16
#include <stdbool.h> // for access to boolean
#include "MMU.h" // MMU outline
#include "GB CPU.h" // for read write

void MMU_reset(struct MMU* mmu){// reset variables

    //clears wram
    uint8_t* wramptr = mmu->wram[0];
    memset(wramptr, 0, 8192);

    //clears zram
    uint8_t* zramptr = mmu->zram[0];
    memset(zramptr, 0, 127);

    mmu->inbios = 1; // sets bios to run 
    mmu->ie = 0; // set flag to no
    mmu->_IF = 0; // set flag to no
    mmu->carttype = 0; // set no cart type

    mmu->mbc.rombank = 0; // reset rom bank
    mmu->mbc.rambank = 0; // reset ram bank
    mmu->mbc.mode = 0; // set controller mode to default
    mmu->mbc.ramon = 0; // turns ram off

    mmu->romoffs = 0x4000; // sets the rom offset to the beginning of the second rom bank to prepare for bank switching
    mmu->ramoffs = 0; // resets the ram offset.
}

bool MMU_load(struct MMU* mmu, const char* filepath){
    
    //establish file stream, check to make sure it worked
    FILE* b = fopen(filepath, "rb");
    if(b == NULL){
        return false;
    }
    
    //find file size, reset pointer
    fseek(b, 0, SEEK_END);
    long fsize = ftell(b);
    rewind(b);
    
    //Malloc, set rom, fill rom, check for null, check for mismatch
    uint8_t* memptr = malloc(fsize); // malloc allocates memory equal to size passed in, returns a pointer to beginning of that memory
    if(memptr == NULL){
        fclose(b);
        return false; // if pointer is NULL, error out
    }
    mmu->rom = memptr; //establish rom pointer to be the beginning of the allocated memory
    size_t check = fread(mmu->rom, 1, fsize, b);//load rom with the gb file and establish a check variable
    if(check != fsize || check == 0){
        fclose(b);
        free(memptr);
        return false;
    }

    //find carttype bits, load it into carttype
    fseek(b, 0x0147, SEEK_SET);
    mmu->carttype = fgetc(b);
    rewind(b);

    //success, end
    fclose(b);
    return true;
}

uint8_t MMU_rb(struct MMU* mmu, uint16_t addr, struct GB_CPU* cpu){
    switch (addr&0xF000)
    {
        //bios and some
        case 0x0000:
            if(mmu->inbios){
                if(addr<0x0100){ return bios[addr]; }
                else if (cpu->_r.pc == 0x0100){ mmu->inbios = 0; return(mmu->rom[addr]);}
                else{
                    return(mmu->rom[addr]);
                }
            }
            else{
                return(mmu->rom[addr]);
            }
            break;
        case 0x1000: 
        case 0x2000: 
        case 0x3000:
            return(mmu->rom[addr]);

        //Rom Bank 1
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            return(mmu->rom[mmu->romoffs+(addr&0x3FFF)]);

        //Vram
        case 0x8000:
        case 0x9000:
            return 0xFF; // Needs to be implemented later, IS gpu

        // External Ram
        case 0xA000:
        case 0xB000:
            return(mmu->eram[mmu->ramoffs+(addr&0x1FFF)]);

        //Work RAM and Echo
        case 0xC000:
        case 0xD000:
        case 0xE000:
            return(mmu->wram[addr&0x1FFF]);

        //
        case 0xF000:
            switch(addr&0x0F00){
                case 0x000: 
                case 0x100: 
                case 0x200: 
                case 0x300:
                case 0x400: 
                case 0x500: 
                case 0x600: 
                case 0x700:
                case 0x800: 
                case 0x900: 
                case 0xA00: 
                case 0xB00:
                case 0xC00: 
                case 0xD00:
                    return(mmu->wram[addr&0x1FFF]);

                //OAM smth smth gpu stuff im not working on yet
                case 0xE00:
                return 0xFF;

                //noy able to work on this yet either
                case 0xF00:
                return 0xFF;    
            }
    }
}

uint16_t MMU_rw(struct MMU* mmu, uint16_t addr, struct GB_CPU* cpu){
    //RW is just a 16 bit read, so its the combination of rb and rb plus 1
    return((uint16_t)(MMU_rb(mmu, addr, cpu) + MMU_rb(mmu,addr+1,cpu)<<8));
}

void MMU_wb(struct MMU* mmu, uint16_t addr, uint8_t val){
// The val variable is a stand in for whatever value will be passed in to be written.
    switch(addr&0xF000){
        // we use a switch statement so that we can evaluate multiple cases at the same time like below:

        // This set of cases will trigger if the address is anywhere between 0x0000 and 0x1FFF inclusive.
        case 0x0000:
        case 0x1000:

            // the purpose of this switch is to turn on eram
            switch(mmu->carttype){
                case 1:
                if((val&0xF) == 0xA){ //0xA is hex for 10 which is a specific value the GB cpu looks for to turn on ram
                    mmu->mbc.ramon = 1;
                    break;
                }
                else{
                    mmu->mbc.ramon = 0;
                    break;
                }
            }
            break;

        //will trigger if between 0x2000 and 0x3FFF inclusive
        case 0x2000:
        case 0x3000:

            switch(mmu->carttype){
                case 1:
                    mmu->mbc.rombank &= 0x60;
                    val &= 0x1F;
                    if(!val){
                        val = 1;
                    }
                    mmu->mbc.rombank |= val;
                    mmu->romoffs = mmu->mbc.rombank * 0x4000;
                break;
            }
            break;


        // RAMbank switching and rombank bit setting
        case 0x4000:
        case 0x5000:

            switch(mmu->carttype){
                case 1:
                    if(mmu->mbc.mode){
                        mmu->mbc.rambank = (val & 0x3); // sets val to 2 bits
                        mmu->ramoffs = mmu->mbc.rambank * 0x2000; // sets offset, ram offset is smaller
                        break;
                    }
                    else{
                        mmu->mbc.rombank &= 0x1F; // masks the rombank to free up bit 5 and 6
                        mmu->mbc.rombank |= ((val & 0x3)<<5); // sets val to 2 bits and then shifts them 5 bits
                        mmu->romoffs = mmu->mbc.rombank * 0x4000; // sets offset
                        break;
                    }
            }
            break;

        //Mode setting
        case 0x6000:
        case 0x7000:

            switch(mmu->carttype){
                case 1:
                    mmu->mbc.mode = (val & 0x1);
                    break;
            }
            break;

        //Vram come back to with gpu
        case 0x8000:
        case 0x9000:
            break;

        //eram
        case 0xA000:
        case 0xB000:
            switch(mmu->carttype){
                case 1:
                    mmu->eram[mmu->ramoffs + (addr & 0x1FFF)] = val;
                    /*
                    this sets the value of val into the provided address after accounting for banks
                    */
                   break;
            }
            break;

        //wram and echo
        case 0xC000:
        case 0xD000:
        case 0xE000:
            mmu->wram[(addr & 0x1FFF)] = val;
            /*
            sets the value of val into address mapping the value of address into proper format
            */
            break;

        //echo, oam, zeropage, I/O, interrupts
        case 0xF000:
            switch(addr&0x0F00){

                //echo ram
                // does the same thing as wram bc its a mirror
                case 0x000: 
                case 0x100: 
                case 0x200: 
                case 0x300:
                case 0x400: 
                case 0x500: 
                case 0x600: 
                case 0x700:
                case 0x800: 
                case 0x900: 
                case 0xA00: 
                case 0xB00:
                case 0xC00: 
                case 0xD00:
                    mmu->wram[(addr & 0x1FFF)] = val;
                    /*
                    sets the value of val into address mapping the value of address into proper format
                    */
                    break;
                
                // OAM
                case 0xE00:
                    // no gpu implementation yet
                    break;

                //Zeropage, I/O, Interupts
                case 0xF00:
                    // no IO implemnentation yet
                    break;

            }
    }
}

void MMU_ww(struct MMU* mmu, uint16_t addr, uint8_t val){
    MMU_wb(mmu,addr,val & 255);
    MMU_wb(mmu,addr+1,val << 8);
}