/*
Start date: 06/23/2025
Last updated: 07/10/2025
Name: Rhett Ward
Update log:
    *06/23/2025
        Created File
        created registers struct
        created clock struct
        created cpu struct
        began writing opcodes
        fixed clock
        fixed unidentified errors
        fixed flag clearing / overloading
        wrote all non MMU ADD opcodes
        wrote the first SUB opcode
    *07/07/2025
        Rewriting Flag system
        working through more opcodes
        defined flags
        wrote a universal ADD(a, r8)
        tested universal ADD_ar8 (it worked)
        reworked all adds to properly handle flags
        reworked comments (needs further depth)
    *07/24/2025
        implemented ADD(A, HL)
        implemented ADD HL's
        implemented ADD (A, n)
        implemented ADD (SP, d)
        finished all ADDs
    *07/25/2025
        implemented SUB(A,r8)
        implemented SUB(A,HL)
        implemented SUB(A,n)
        finished all SUBs
        implemented AND(A,r8)
        implemented AND(A,HL)
        implemented AND(A,n)
        finished all ANDs
    *
        plenty of unnoted one or two opcode writes spread out
    *08/27/2025
        Load opcodes finished
    *08/28/2025
        Compare opcodes
        CPL opcode
        DAA opcode
    * August 28 - October 2nd
        Updates tracked Via git hub comments
    * 10/02/2025
        All Bit shift op codes
        Finished Jumps excluding RETI and RST vec
        Started switch statement that actually allows the cpu to run
*/


/* Summary of file:
Gameboy CPU, this is where all the opcodes are, where the flags are defined, registers, cpu clock, etc
*/

//Includes needed
#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16 
#include "MMU.h" // memory manipulation unit interface
#include "GB CPU.h" // blueprint for this file 

struct GB_CPU GlobalCPU; //CPU var that all the ops point to

void initialize(){ //set cpu up, kinda like turning the gameboy on
    GlobalCPU._r.a = 0;
    GlobalCPU._r.b = 0;
    GlobalCPU._r.c = 0;
    GlobalCPU._r.d = 0;
    GlobalCPU._r.e = 0;
    GlobalCPU._r.f = 0;
    GlobalCPU._r.h = 0;
    GlobalCPU._r.l = 0;

    GlobalCPU._r.pc = 0;
    GlobalCPU._r.sp = 0;
    GlobalCPU._r.ime = 0;

    GlobalCPU._c.m = 0;
    GlobalCPU._c.t = 0;
}

int pending_ei;
int stop_var = 1;


//opcodes / instruction functions:
#pragma region OPcodes

#pragma region LD functions

/**
 *@brief Copy the value of n into r8 AKA LD(r8,n)
 *@param cpu pointer to GB CPU
 *@param r8 pointer to register 
 */
void LD_nr8(struct GB_CPU* cpu, uint8_t* r8){

    uint8_t n = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);

    *r8 = n; // this should replace the value pointed to by r8 with the value of n

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; // iterate past instruction and value
}

/**
 *@brief Copy the value of r82 into register pointed to by r8
 *@param cpu pointer to GB CPU
 *@param r8 pointer to register
 *@param r82 pointer to register
 */
void LD_r8(struct GB_CPU* cpu, uint8_t* r8, uint8_t r82){

    *r8 = r82; // this should replace the value pointed to by r8 with the value of n

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 *@brief Copy the value of n into BC AKA LD(BC,nn)
 *@param cpu pointer to GB CPU
 */
void LD_BC(struct GB_CPU* cpu){
    
    cpu->_r.c = MMU_rb(&cpu->mmu, (cpu->_r.pc+1), cpu);
    cpu->_r.b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 2), cpu);
    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value

}

/**
 *@brief Copy the value of n into DE AKA LD(DE,nn)
 *@param cpu pointer to GB CPU
 */
void LD_DE(struct GB_CPU* cpu){
    
    cpu->_r.d = MMU_rb(&cpu->mmu, (cpu->_r.pc+1), cpu);
    cpu->_r.e = MMU_rb(&cpu->mmu, (cpu->_r.pc + 2), cpu);
    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value

}

/**
 *@brief Copy the value of n into HL AKA LD(HL,nn)
 *@param cpu pointer to GB CPU
 */
void LD_HL(struct GB_CPU* cpu){
    
    cpu->_r.h = MMU_rb(&cpu->mmu, (cpu->_r.pc+1), cpu);
    cpu->_r.l = MMU_rb(&cpu->mmu, (cpu->_r.pc + 2), cpu);
    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value

}

/**
 * @brief copy the value held in the register r8 into the address pointed to by HL AKA LD(HL,r8)
 * @param cpu Pointer to the cpu
 * @param r8 8 bit register value
 */
void LD_HLr8(struct GB_CPU* cpu, uint8_t r8){
    MMU_wb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), r8);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value of n8 into the address pointed to by HL AKA LD(HL,n8)
 * @param cpu Pointer to the cpu
 */
void LD_HLn(struct GB_CPU* cpu){
    MMU_wb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu));

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //incrememnt past instruction and value
}

/**
 * @brief copy the value held in the register HL into the address pointed to by r8 AKA LD(r8,HL)
 * @param cpu Pointer to the cpu
 * @param r8 8 bit register value
 */
void LD_r8HL(struct GB_CPU* cpu, uint8_t r8){
    MMU_wb(&cpu->mmu, r8, ((cpu->_r.h<<8) + cpu->_r.l));

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value in register A into the byte pointed to by r16 (r8 +r82) AKA:LD(r16, A)
 * @param cpu Pointer to the cpu
 * @param r8 8 bit register value, upper bit of r16
 * @param r82 8  bit register value, lower bit of r16
 */
void LD_r16A(struct GB_CPU* cpu, uint8_t r8, uint8_t r82){
    uint16_t r16 = ((r8<<8) + r82);
    MMU_wb(&cpu->mmu, r16, cpu->_r.a);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value pointed to by r16 (r8 +r82) into register A AKA:LD(r16, A)
 * @param cpu Pointer to the cpu
 * @param r8 8 bit register value, upper bit of r16
 * @param r82 8  bit register value, lower bit of r16
 */
void LD_Ar16(struct GB_CPU* cpu, uint8_t r8, uint8_t r82){
    uint8_t r8 = MMU_rb(&cpu->mmu, ((r8<<8) + r82), cpu);
    MMU_wb(&cpu->mmu,cpu->_r.a, r8);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value in register A into the byte pointed to by n16 (pc +1  + pc + 2) AKA:LD(n16, A)
 * @param cpu Pointer to the cpu
 */
void LD_n16A(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu));
    MMU_wb(&cpu->mmu, r16, cpu->_r.a);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value
}

/**
 * @brief copy the value in register A into the byte pointed to by pc+1 + 0xFF00 if its between $$FF00 $FFFF AKA:LDH(n16, A)
 * @param cpu Pointer to the cpu
 */
void LDH_n16A(struct GB_CPU* cpu){
    uint16_t r16 = ( 0xFF00 +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu));

    if(r16 >= 0xFF00 && r16 <= 0xFFFF){
        MMU_wb(&cpu->mmu, r16, cpu->_r.a);
    }

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //incrememnt past instruction and value
}

/**
 * @brief copy the value read from n16 into register A given its between 0xFF00 and 0xFFFF AKA:LDH(A, n16)
 * @param cpu Pointer to the cpu
 */
void LDH_An16(struct GB_CPU* cpu){
    uint8_t r16 = ( 0xFF00 +  MMU_rb(&cpu->mmu, (cpu->_r.pc+1<<8) + (cpu->_r.pc+2), cpu));

    if(r16 >= 0xFF00 && r16 <= 0xFFFF){
        MMU_wb(&cpu->mmu, cpu->_r.a, r16);
    }

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value
}

/**
 * @brief Copy the value at register A into $FF00 + C  AKA:LDH(C,A)
 * @param cpu Pointer to the cpu
 */
void LDHC (struct GB_CPU* cpu){
    MMU_wb(&cpu->mmu, cpu->_r.c + 0xFF00, cpu->_r.a);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief Copy the value at register A into $FF00 + C  AKA:LDH(C,A)
 * @param cpu Pointer to the cpu
 */
void LDHCu8 (struct GB_CPU* cpu){
    uint8_t u8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu);
    MMU_wb(&cpu->mmu, u8 + 0xFF00, cpu->_r.a);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //incrememnt past instruction and value
}

/**
 * @brief Copy the value at $FF00 + C into register A AKA:LDH(A,C)
 * @param cpu Pointer to the cpu
 */
void LDHAC (struct GB_CPU* cpu){
    MMU_wb(&cpu->mmu, cpu->_r.a, cpu->_r.c + 0xFF00);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value held in the register A into the address pointed to by HL then increment HL AKA LD(HLI,A)
 * @param cpu Pointer to the cpu
 */
void LD_HLIA(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL

    MMU_wb(&cpu->mmu, r16, cpu->_r.a); // write A to that byte
    r16++; // Increment HL

    cpu->_r.h = (r16>>8) & 255; // store High byte in h 
    cpu->_r.l = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the value held in the register A into the address pointed to by HL then decrement HL AKA LD(HLD,A)
 * @param cpu Pointer to the cpu
 */
void LD_HLDA(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL

    MMU_wb(&cpu->mmu, r16, cpu->_r.a); // write A to that byte
    r16--; // Increment HL

    cpu->_r.h = (r16>>8) & 255; // store High byte in h 
    cpu->_r.l = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the address pointed to by HL into A then decrement HL AKA LD(A,HLD)
 * @param cpu Pointer to the cpu
 */
void LD_AHLD(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL

    cpu->_r.a = MMU_rb(&cpu->mmu, r16, cpu);
    r16--; // Increment HL

    cpu->_r.h = (r16>>8) & 255; // store High byte in h 
    cpu->_r.l = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief copy the address pointed to by HL into A then increment HL AKA LD(A,HLI)
 * @param cpu Pointer to the cpu
 */
void LD_AHLI(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL

    cpu->_r.a = MMU_rb(&cpu->mmu, r16, cpu);
    r16++; // Increment HL

    cpu->_r.h = (r16>>8) & 255; // store High byte in h 
    cpu->_r.l = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief Copy the value held at n16 into register sp AKA LD(SP, n16)
 * @param cpu Pointer to the cpu
 */
void LD_SPn16(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu));

    //MMU_wb(&cpu->mmu, cpu->_r.sp, r16);
    cpu->_r.sp = r16;

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value
}

/**
 * @brief copy SP & 0xFF at address n16 and SP >> 8 at address n16 + 1
 * @param cpu pointer to cpu
 */
void LD_n16SP(struct GB_CPU* cpu){

    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu));
    MMU_wb(&cpu->mmu, (r16+1), cpu->_r.sp >> 8);
    MMU_wb(&cpu->mmu, r16, (cpu->_r.sp & 0xFF));

    cpu->_r.m = 5; cpu->_r.t = 20; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 3; //incrememnt past instruction and value
}

/**
 * @brief  Add the signed value e8 (d) to SP and copy the result into HL
 * @param cpu pointer to cpu
 */
void LD_HLspe8(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL


    // Start of copied code from ADD
    uint32_t i = 0;
    int8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);  // reads the very next byte of memory to find the value needed to add to a
    uint16_t a = cpu->_r.sp;

    i = a + b;

    cpu->_r.f &= ~N_FLAG;

    cpu->_r.f &= ~Z_FLAG;

    uint8_t hc = ((a & 0x0F) + (b & 0x0F));

    if(hc > 15){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(((a & 0xFF) + (uint8_t)b) > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }
    // End of copied code from ADD

    r16 = (uint16_t) i;

    cpu->_r.h = r16>>8;
    cpu->_r.l = r16 & 0xFF;

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //incrememnt past instruction and value

}

/**
 * @brief Copies register HL into register SP AKA LD(sp, hl)
 * @param cpu Pointer to CPU
 */
void LD_HLSP(struct GB_CPU* cpu){

    uint16_t r16 = ((cpu->_r.h<<8) + cpu->_r.l); // get the byte pointer held by HL

    cpu->_r.sp = r16;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * I FINISHED LOADS 8/27/2025
 */


 #pragma endregion LD functions

#pragma region Arithmetic Instructions

#pragma region ADD functions

/**
 * @brief Univeral Function that acts as any ADD(A,r8) opcode (r8 standing for 8 bit register)
 * @param cpu Pass in of a pointer to the CPU
 * @param reg Pass in of a pointer to a 8 bit reg
 */
void ADD_ar8(struct GB_CPU* cpu, uint8_t r8){
    uint16_t i = 0;
    uint8_t b = r8;
    uint8_t a = cpu->_r.a;

    i = a + b;

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    
    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //increment instruction pointer
}

/**
 * @brief Add HL to A, Leaves result in A AKA (ADD A, HL)
 * @param cpu point to GB_CPU
 */
void ADD_HL(struct GB_CPU* cpu){

    uint16_t i = 0;
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    i = a + b;

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Add BC to HL, Leaves result in HL AKA (ADD HL, BC)
 * @param cpu point to GB_CPU
 */
void ADDHLBC(struct GB_CPU* cpu){
    
    uint16_t HL = (cpu->_r.h<<8) + cpu->_r.l; // create a variable HL that combines the two 8 bit registers h and l
    uint16_t BC = (cpu->_r.b<<8) + cpu->_r.c; // create a variable BC that combines the two 8 bit registers b and c

    uint32_t i = HL + BC; // total that allows overflow
    
    cpu->_r.f &= ~N_FLAG; // Clears Subtraction flag

    if(i > 65535){
        cpu->_r.f |= C_FLAG; //sets carry flag if overflow
    }
    else{
        cpu->_r.f &= ~C_FLAG; // performs bitwise and with a bitflipped carry flag to clear carry
    }

    // not rewriting my functional and correct hc check but could also be written if ((HL^ BC ^ i) & 0x1000) {}

    if(((HL & 4095) + (BC & 4095)) > 4095){ //
        cpu->_r.f |= H_FLAG; // sets half carry if there is overflow on the lower bits
    }
    else{
        cpu->_r.f &= ~H_FLAG; // performs bitwise and with a bitflipped half carry flag to clear half carry
    }

    cpu->_r.h = (i>>8) & 255; // store High byte in h 
    cpu->_r.l = i & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Add DE to HL, Leaves result in HL AKA (ADD HL, DE)
 * @param cpu point to GB_CPU
 */
void ADDHLDE(struct GB_CPU* cpu){
    
    uint16_t HL = (cpu->_r.h<<8) + cpu->_r.l; // create a variable HL that combines the two 8 bit registers h and l
    uint16_t DE = (cpu->_r.d<<8) + cpu->_r.e; // create a variable DE that combines the two 8 bit registers b and c

    uint32_t i = HL + DE; // total that allows overflow
    
    cpu->_r.f &= ~N_FLAG; // Clears Subtraction flag

    if(i > 65535){
        cpu->_r.f |= C_FLAG; //sets carry flag if overflow
    }
    else{
        cpu->_r.f &= ~C_FLAG; // performs bitwise and with a bitflipped carry flag to clear carry
    }

    if(((HL & 4095) + (DE & 4095)) > 4095){ //
        cpu->_r.f |= H_FLAG; // sets half carry if there is overflow on the lower bits
    }
    else{
        cpu->_r.f &= ~H_FLAG; // performs bitwise and with a bitflipped half carry flag to clear half carry
    }

    cpu->_r.h = (i>>8) & 255; // store High byte in h 
    cpu->_r.l = i & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Add HL to HL, Leaves result in HL AKA (ADD HL, HL)
 * @param cpu point to GB_CPU
 */
void ADDHLHL(struct GB_CPU* cpu){
    
    uint16_t HL = (cpu->_r.h<<8) + cpu->_r.l; // create a variable HL that combines the two 8 bit registers h and l
    

    uint32_t i = HL + HL; // total that allows overflow
    
    cpu->_r.f &= ~N_FLAG; // Clears Subtraction flag

    if(i > 65535){
        cpu->_r.f |= C_FLAG; //sets carry flag if overflow
    }
    else{
        cpu->_r.f &= ~C_FLAG; // performs bitwise and with a bitflipped carry flag to clear carry
    }

    if(((HL & 4095) + (HL & 4095)) > 4095){ //
        cpu->_r.f |= H_FLAG; // sets half carry if there is overflow on the lower bits
    }
    else{
        cpu->_r.f &= ~H_FLAG; // performs bitwise and with a bitflipped half carry flag to clear half carry
    }

    cpu->_r.h = (i>>8) & 255; // store High byte in h 
    cpu->_r.l = i & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Add SP to HL, Leaves result in HL AKA (ADD HL, SP)
 * @param cpu point to GB_CPU
 */
void ADDHLSP(struct GB_CPU* cpu){
    
    uint16_t HL = (cpu->_r.h<<8) + cpu->_r.l; // create a variable HL that combines the two 8 bit registers h and l
    

    uint32_t i = HL + cpu->_r.sp; // total that allows overflow
    
    cpu->_r.f &= ~N_FLAG; // Clears Subtraction flag

    if(i > 65535){
        cpu->_r.f |= C_FLAG; //sets carry flag if overflow
    }
    else{
        cpu->_r.f &= ~C_FLAG; // performs bitwise and with a bitflipped carry flag to clear carry
    }

    if(((HL & 4095) + (cpu->_r.sp & 4095)) > 4095){ //
        cpu->_r.f |= H_FLAG; // sets half carry if there is overflow on the lower bits
    }
    else{
        cpu->_r.f &= ~H_FLAG; // performs bitwise and with a bitflipped half carry flag to clear half carry
    }

    cpu->_r.h = (i>>8) & 255; // store High byte in h 
    cpu->_r.l = i & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Add n to A, Leaves result in A AKA (ADD A, n)
 * @param cpu point to GB_CPU
 */
void ADD_n2a(struct GB_CPU* cpu){
    
    uint16_t i = 0;
    uint8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);  // reads the very next byte of memory to find the value needed to add to a
    uint8_t a = cpu->_r.a;

    i = a + b;

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc+=2; // increment counter past instruction and past value of n

}

/**
 * @brief Add d to SP, Leaves result in SP AKA (ADD SP, d)
 * @param cpu point to GB_CPU
 */
void ADD_SP2d(struct GB_CPU* cpu){

    uint32_t i = 0;
    int8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);  // reads the very next byte of memory to find the value needed to add to a
    uint16_t a = cpu->_r.sp;

    i = a + b;

    cpu->_r.f &= ~N_FLAG;

    cpu->_r.f &= ~Z_FLAG;

    uint8_t hc = ((a & 0x0F) + (b & 0x0F));

    if(hc > 15){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(((a & 0xFF) + (uint8_t)b) > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.sp = i; //sets result to SP
    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc+=2; // increment counter past instruction and past value of n
}

/**
 * @brief adds the value in r8 plus the carry flag to A
 * @param cpu pointer to cpu
 * @param r8 pointer to 8 bit register
 */
void ADC_ar8(struct GB_CPU* cpu, uint8_t r8){

    uint16_t i = 0;
    uint8_t b = r8;
    uint8_t a = cpu->_r.a;
    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a + b + c; // total (ADC)

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG; // half carry check

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //increment instruction pointer
}

/**
 * @brief add the byte pointed to by HL plus the carry flag to A
 * @param cpu pointer to the cpu
 */
void ADC_hl(struct GB_CPU* cpu){
    uint16_t i = 0;
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a + b + c;

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief adds the value in n8 plus the carry flag to A
 * @param cpu pointer to cpu
 */
void ADC_an8(struct GB_CPU* cpu){

    uint16_t i = 0;
    uint8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);  // reads the very next byte of memory to find the value needed to add to a
    uint8_t a = cpu->_r.a;
    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a + b + c; // total (ADC)

    cpu->_r.f &= ~N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG; // half carry check

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //increment instruction pointer
}

#pragma endregion ADD funtions

#pragma region SUB functions

/**
 * @brief Universal subtract r8 from a, store result in a
 * @param cpu pointer for GB CPU
 */
void SUB_ar8(struct GB_CPU* cpu, uint8_t r8){

    int16_t i = 0; //temp variable
    uint8_t b = r8;

    i = cpu->_r.a - b;

    cpu->_r.f = N_FLAG;

    if(i < 0){ // underflow check
        cpu->_r.f |= C_FLAG; // half carry plus some other flag
    }
    else{
        cpu->_r.f &= ~C_FLAG; // subtraction flag
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // zero flag
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if((cpu->_r.a ^ r8 ^ i) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Subtract HL from A, Leaves result in A AKA (SUB A, HL)
 * @param cpu point to GB_CPU
 */
void SUB_HL(struct GB_CPU* cpu){

    int16_t i = 0;
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    i = a - b;

    cpu->_r.f = N_FLAG;

    uint8_t hc = (a ^ b ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i < 0){
        cpu->_r.f |= C_FLAG; //if underflow happened, add underflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Subtract N from A leaves result in A AKA (SUB A, n)
 * @param cpu pointer for GB CPU
 */
void SUB_an(struct GB_CPU* cpu){

    int16_t i = 0; //temp variable
    uint8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc +1), cpu);

    i = cpu->_r.a - b;

    cpu->_r.f = N_FLAG;

    if(i < 0){ // underflow check
        cpu->_r.f |= C_FLAG; // half carry plus some other flag
    }
    else{
        cpu->_r.f &= ~C_FLAG; // subtraction flag
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // zero flag
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if((cpu->_r.a ^ b ^ i) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2; // move past instruction and value
}

/**
 * @brief Subtracts the value in r8 plus the carry flag from A
 * @param cpu pointer to cpu
 * @param r8 pointer to 8 bit register
 */
void SBC_ar8(struct GB_CPU* cpu, uint8_t r8){

    uint16_t i = 0;
    uint8_t b = r8;
    uint8_t a = cpu->_r.a;
    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a - (b + c); // total (ADC)

    cpu->_r.f |= N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG; // half carry check

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //increment instruction pointer
}

/**
 * @brief subtracts the byte pointed to by HL plus the carry flag from A
 * @param cpu pointer to the cpu
 */
void SBC_hl(struct GB_CPU* cpu){
    uint16_t i = 0;
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a - (b + c);

    cpu->_r.f |= N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Subtracts the value in n8 plus the carry flag from A
 * @param cpu pointer to cpu
 */
void SBC_an8(struct GB_CPU* cpu){

    uint16_t i = 0;
    uint8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu);  // reads the very next byte of memory to find the value needed to add to a
    uint8_t a = cpu->_r.a;
    uint8_t c; // holds value of carry flag

    if ((cpu->_r.f & C_FLAG) != 0){ //checks carry flag 
        c = 1; // if carry flag
    }else{
        c = 0; // if not carry flag
    }

    i = a - (b + c); // total (ADC)

    cpu->_r.f |= N_FLAG;

    uint8_t hc = (a ^ b ^ c ^ i) & C_FLAG; // half carry check

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //if overflow happened, add overflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; //increment instruction pointer
}

#pragma endregion SUB functions

#pragma region INC/DEC

/**
 * @brief Decrement value in r8 by 1
 * @param cpu pointer to cpu
 * @param r8 pointer to 8 bit register
 */
void DECr8(struct GB_CPU* cpu, uint8_t* r8){
    *r8 = *r8 - 1;
    
    if(*r8 == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f |= N_FLAG;

    if(((*r8 + 1) ^ (*r8)) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 *@brief Decrement the byte pointed to by HL by 1
 *@param cpu pointer to cpu 
 */
void DECHL(struct GB_CPU* cpu){
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    b--;
    MMU_wb(&cpu->mmu,((cpu->_r.h<<8) + cpu->_r.l), b);

    if(b == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f |= N_FLAG;

    if(((b+ 1) ^ (b)) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Decrement the value in register r16 by 1
 * @param cpu pointer to the cpu
 * @param r8 pointer to 8 bit register
 * @param r82 pointer to 8 bit register number 2
 */
void DECr16(struct GB_CPU* cpu, uint8_t* r8, uint8_t* r82){
    uint16_t r16 = (((*r8)<<8) + *r82);
    r16--;
    *r8 = (r16>>8) & 255; // store High byte in h 
    *r82 = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Decrement the value 
 * @param cpu pointer to the cpu
 */
void DECsp(struct GB_CPU* cpu){
    cpu->_r.sp--;

    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Increments the value by 1
 * @param cpu pointer to the cpu
 * @param r8 pointer to register 8
 */
void INCr8(struct GB_CPU* cpu, uint8_t* r8){
    (*r8)++;


    cpu->_r.f &= ~N_FLAG;

    if(*r8 == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(((*r8 - 1) ^ (*r8)) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Increment value by 1
 * @param cpu pointer to cpu
 */
void INCHL(struct GB_CPU* cpu){

    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu));// assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    b++;
    MMU_wb(&cpu->mmu,((cpu->_r.h<<8) + cpu->_r.l), b);

    cpu->_r.f &= ~N_FLAG;

    if(b == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(((b - 1) ^ (b)) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Increment the value in register r16 by 1
 * @param cpu pointer to the cpu
 * @param r8 pointer to 8 bit register
 * @param r82 pointer to 8 bit register number 2
 */
void INCr16(struct GB_CPU* cpu, uint8_t* r8, uint8_t* r82){
    uint16_t r16 = (((*r8)<<8) + *r82);
    r16++;
    *r8 = (r16>>8) & 255; // store High byte in h 
    *r82 = r16 & 255; // store low byte in l

    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Increment the value 
 * @param cpu pointer to the cpu
 */
void INCsp(struct GB_CPU* cpu){
    cpu->_r.sp++;

    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

#pragma endregion INC/DEC

#pragma region Compare functions

/**
 * @brief Compare A r8
 * @param cpu pointer for GB CPU
 */
void CP_ar8(struct GB_CPU* cpu, uint8_t r8){

    int16_t i = 0; //temp variable
    uint8_t b = r8;

    i = cpu->_r.a - b;

    cpu->_r.f = N_FLAG;

    if(i < 0){ // underflow check
        cpu->_r.f |= C_FLAG; // half carry plus some other flag
    }
    else{
        cpu->_r.f &= ~C_FLAG; // subtraction flag
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // zero flag
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if((cpu->_r.a ^ r8 ^ i) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief Compare A[HL]
 * @param cpu point to GB_CPU
 */
void CP_HL(struct GB_CPU* cpu){

    int16_t i = 0;
    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    i = a - b;

    cpu->_r.f = N_FLAG;

    uint8_t hc = (a ^ b ^ i) & C_FLAG;

    if(hc != 0){
        cpu->_r.f |= H_FLAG;
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if i = 0 set f to 0, 0x80 is the zero denotation, the if checks if the result of the math is a value b/t 1-255 if not then proceed.
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if(i < 0){
        cpu->_r.f |= C_FLAG; //if underflow happened, add underflow flag to flag stack
    }
    else{
        cpu->_r.f &= ~C_FLAG;
    }

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}
 
/**
 * @brief Compare A n8
 * @param cpu pointer for GB CPU
 */
void CP_an(struct GB_CPU* cpu){

    int16_t i = 0; //temp variable
    uint8_t b = MMU_rb(&cpu->mmu, (cpu->_r.pc +1), cpu);

    i = cpu->_r.a - b;

    cpu->_r.f = N_FLAG;

    if(i < 0){ // underflow check
        cpu->_r.f |= C_FLAG; // half carry plus some other flag
    }
    else{
        cpu->_r.f &= ~C_FLAG; // subtraction flag
    }

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // zero flag
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    if((cpu->_r.a ^ b ^ i) & C_FLAG){
        cpu->_r.f |= H_FLAG; // halfcarry flag
    }
    else{
        cpu->_r.f &= ~H_FLAG;
    }

    cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2; // move past instruction and value
}

#pragma endregion Compare functions

#pragma endregion Arithmetic

#pragma region Bitwise Logic Instructions

#pragma region And

/**
 * @brief bitwise and of A and r8 and stores in A AKA AND(A, r8)
 * @param cpu pointer to GB CPU
 */
void AND_ar8(struct GB_CPU* cpu, uint8_t r8){

    cpu->_r.a &= r8; // bitwise comparison

    cpu->_r.f &= ~N_FLAG;

    if (cpu->_r.a == 0) {
        cpu->_r.f |= Z_FLAG; // if = 0 then set 0 flag
    } else {
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f |= H_FLAG; // sets H_flag bc docs say so

    cpu->_r.f &= ~C_FLAG; // clears C_flag bc docs say so


    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief bitwise and of A and value pointed to by HL and stores in A AKA AND(A, HL)
 * @param cpu point to GB_CPU
 */
void AND_HL(struct GB_CPU* cpu){

    uint8_t b = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    cpu->_r.a &= b;

    cpu->_r.f &= ~N_FLAG;

    if (cpu->_r.a == 0) {
        cpu->_r.f |= Z_FLAG; // if = 0 then set 0 flag
    } else {
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f |= H_FLAG; // sets H_flag bc docs say so

    cpu->_r.f &= ~C_FLAG; // clears C_flag bc docs say so

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; //incrememnt past instruction
}

/**
 * @brief bitwise and of A and value pointed to by pc+1 and stores in A AKA AND(A, n)
 * @param cpu point to GB_CPU
 */
void AND_an(struct GB_CPU* cpu){

    uint8_t b = (MMU_rb(&cpu->mmu, (cpu->_r.pc + 1), cpu)); // assign uint8_t b the value read out of memory at 16 bit address created by high bit h and low bit l
    uint8_t a = cpu->_r.a;

    cpu->_r.a &= b;

    cpu->_r.f &= ~N_FLAG;

    if (cpu->_r.a == 0) {
        cpu->_r.f |= Z_FLAG; // if = 0 then set 0 flag
    } else {
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f |= H_FLAG; // sets H_flag bc docs say so

    cpu->_r.f &= ~C_FLAG; // clears C_flag bc docs say so

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2; // move past instruction and value
}

#pragma endregion And

#pragma region OR

/**
 * @brief Bitwise or b/t reg A and r8
 * @param cpu pointer to the cpu
 * @param r8 r8 pointer
 */
void ORr8(struct GB_CPU* cpu, uint8_t* r8){
uint8_t i = cpu->_r.a | *r8;
cpu->_r.a |= *r8;

if(i==0){
    cpu->_r.f |= Z_FLAG;
}else{
    cpu->_r.f &= ~Z_FLAG;
}

cpu->_r.f &= ~N_FLAG;
cpu->_r.f &= ~H_FLAG;
cpu->_r.f &= ~C_FLAG;

cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
cpu->_r.pc++;
}

/**
 * @brief Bitwise or b/t reg A and HL
 * @param cpu pointer to the cpu
 */
void ORHL(struct GB_CPU* cpu){
uint8_t HL = (MMU_rb(&cpu->mmu, ((cpu->_r.h<<8) + cpu->_r.l), cpu));
uint8_t i = cpu->_r.a | HL ;
cpu->_r.a |= HL;

if(i==0){
    cpu->_r.f |= Z_FLAG;
}else{
    cpu->_r.f &= ~Z_FLAG;
}

cpu->_r.f &= ~N_FLAG;
cpu->_r.f &= ~H_FLAG;
cpu->_r.f &= ~C_FLAG;

cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
cpu->_r.pc++;
}

/**
 * @brief Bitwise or b/t reg A and n8
 * @param cpu pointer to the cpu
 */
void ORn8(struct GB_CPU* cpu){
uint8_t n8 = (MMU_rb(&cpu->mmu, (cpu->_r.pc+1), cpu));
uint8_t i = cpu->_r.a | n8 ;
cpu->_r.a |= n8;

if(i==0){
    cpu->_r.f |= Z_FLAG;
}else{
    cpu->_r.f &= ~Z_FLAG;
}

cpu->_r.f &= ~N_FLAG;
cpu->_r.f &= ~H_FLAG;
cpu->_r.f &= ~C_FLAG;

cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
cpu->_r.pc+=2;
}

#pragma endregion OR

#pragma region XOR

/**
 * @brief XOR A & r8
 * @param cpu pointer to the cpu
 * @param r8 pointer to 8 bit register
 */
void XORr8(struct GB_CPU* cpu, uint8_t* r8){

    uint8_t tempor = cpu->_r.a | *r8;
    uint8_t tempand = cpu->_r.a & *r8;
    uint8_t i = tempor & ~tempand;
    cpu->_r.a = i;

    if(i==0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~H_FLAG;
    cpu->_r.f &= ~C_FLAG;
    cpu->_r.f &= ~N_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;

}

/**
 * @brief XOR A & HL
 * @param cpu pointer to the cpu
 */
void XORHL(struct GB_CPU* cpu){

    uint8_t r8 = MMU_rb(&cpu->mmu, ((cpu->_r.h << 8)+ cpu->_r.l), cpu);
    uint8_t tempor = cpu->_r.a | r8;
    uint8_t tempand = cpu->_r.a & r8;
    uint8_t i = tempor & ~tempand;
    cpu->_r.a = i;

    if(i==0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~H_FLAG;
    cpu->_r.f &= ~C_FLAG;
    cpu->_r.f &= ~N_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
    
}

/**
 * @brief XOR A & n8
 * @param cpu pointer to the cpu
 */
void XORn8(struct GB_CPU* cpu){
    uint8_t r8 = MMU_rb(&cpu->mmu,(cpu->_r.pc + 1),cpu);
    uint8_t tempor = cpu->_r.a | r8;
    uint8_t tempand = cpu->_r.a & r8;
    uint8_t i = tempor & ~tempand;
    cpu->_r.a = i;

    if(i==0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~H_FLAG;
    cpu->_r.f &= ~C_FLAG;
    cpu->_r.f &= ~N_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc+=2;

}

#pragma endregion XOR

/**
 * @brief Complemnent accumulator (invert A) also called bitwise NOT
 * @param cpu pointer to cpu
 */
void CPL(struct GB_CPU* cpu){
    cpu->_r.a = ~cpu->_r.a;

    cpu->_r.f |= N_FLAG;
    cpu->_r.f |= H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

#pragma endregion Bitwise Logic Instructions

#pragma region Bit Flag

/**
 * @brief test bit u3 in r8, set 0 flag if bit didnt set
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 * @param r8 8 bit register pointer
 */
void BIT_u3r8(struct GB_CPU* cpu, uint8_t u3, uint8_t* r8){
    uint8_t i = (u3 &= 00000111);

    uint8_t res;

    switch(i){
        case 0:
            res = (*r8 & 00000001);
            break;
        case 1:
            res = (*r8 & 00000010);
            break;
        case 2:
            res = (*r8 & 00000100);
            break;
        case 3:
            res = (*r8 & 00001000);
            break;
        case 4:
            res = (*r8 & 00010000);
            break;
        case 5:
            res = (*r8 & 00100000);
            break;
        case 6:
            res = (*r8 & 01000000);
            break;
        case 7:
            res = (*r8 & 10000000);
            break;          
    }

    if(res == 00000000){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f |= H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief test bit u3 in HL, set 0 flag if bit didnt set
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 */
void BIT_u3HL(struct GB_CPU* cpu, uint8_t u3){
    uint8_t i = (u3 &= 00000111);

    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t res;

    switch(i){
        case 0:
            res = (r8 & 00000001);
            break;
        case 1:
            res = (r8 & 00000010);
            break;
        case 2:
            res = (r8 & 00000100);
            break;
        case 3:
            res = (r8 & 00001000);
            break;
        case 4:
            res = (r8 & 00010000);
            break;
        case 5:
            res = (r8 & 00100000);
            break;
        case 6:
            res = (r8 & 01000000);
            break;
        case 7:
            res = (r8 & 10000000);
            break;          
    }

    if(res == 00000000){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f |= H_FLAG;

    cpu->_r.m = 3; cpu->_r.t = 12; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief set bit u3 in HL to 0.
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 */
void RES_u3HL(struct GB_CPU* cpu, uint8_t u3){
    uint8_t i = (u3 &= 00000111);

    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);

    switch(i){
        case 0:
            r8 &= ~00000001;
            break;
        case 1:
            r8 &= ~00000010;
            break;
        case 2:
            r8 &= ~00000100;
            break;
        case 3:
            r8 &= ~00001000;
            break;
        case 4:
            r8 &= ~00010000;
            break;
        case 5:
            r8 &= ~00100000;
            break;
        case 6:
            r8 &= ~01000000;
            break;
        case 7:
            r8 &= ~10000000;
            break;          
    }

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief set bit u3 in r8 to 0.
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 * @param r8 8bit register pointer
 */
void RES_u3r8(struct GB_CPU* cpu, uint8_t u3, uint8_t* r8){
    uint8_t i = (u3 &= 00000111);

    switch(i){
        case 0:
            *r8 &= ~00000001;
            break;
        case 1:
            *r8 &= ~00000010;
            break;
        case 2:
            *r8 &= ~00000100;
            break;
        case 3:
            *r8 &= ~00001000;
            break;
        case 4:
            *r8 &= ~00010000;
            break;
        case 5:
            *r8 &= ~00100000;
            break;
        case 6:
            *r8 &= ~01000000;
            break;
        case 7:
            *r8 &= ~10000000;
            break;          
    }

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), *r8);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief set bit u3 in r8 to 1.
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 * @param r8 8bit register pointer
 */
void SET_u3r8(struct GB_CPU* cpu, uint8_t u3, uint8_t* r8){
    uint8_t i = (u3 &= 00000111);

    switch(i){
        case 0:
            *r8 |= 00000001;
            break;
        case 1:
            *r8 |= 00000010;
            break;
        case 2:
            *r8 |= 00000100;
            break;
        case 3:
            *r8 |= 00001000;
            break;
        case 4:
            *r8 |= 00010000;
            break;
        case 5:
            *r8 |= 00100000;
            break;
        case 6:
            *r8 |= 01000000;
            break;
        case 7:
            *r8 |= 10000000;
            break;          
    }

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), *r8);

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief set bit u3 in HL to 1.
 * @param cpu pointer to the cpu
 * @param u3 3bit that says which bit to test
 */
void SET_u3HL(struct GB_CPU* cpu, uint8_t u3){
    uint8_t i = (u3 &= 00000111);

    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);

    switch(i){
        case 0:
            r8 |= 00000001;
            break;
        case 1:
            r8 |= 00000010;
            break;
        case 2:
            r8 |= 00000100;
            break;
        case 3:
            r8 |= 00001000;
            break;
        case 4:
            r8 |= 00010000;
            break;
        case 5:
            r8 |= 00100000;
            break;
        case 6:
            r8 |= 01000000;
            break;
        case 7:
            r8 |= 10000000;
            break;          
    }

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

#pragma endregion Bit Flag

#pragma region Bit Shift

/**
 * @brief shift bits of r8 to the left one and carry bit 7 to bit 0 and carry
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void RLCr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 10000000); // store 7th bit for carry
    *r8 = *r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        *r8 |= 00000001; // set bit 0 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        *r8 &= 11111110; // set bit 0 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the left one and carry bit 7 to bit 0 and carry
 * @param cpu pointer to the cpu
 */
void RLCHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 10000000); // store 7th bit for carry
    r8 = r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        r8 |= 00000001; // set bit 0 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        r8 &= 11111110; // set bit 0 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of A to the left one and carry bit 7 to bit 0 and carry
 * @param cpu pointer to the cpu
 */
void RLCA(struct GB_CPU* cpu){
    uint8_t* r8 = &cpu->_r.a;
    uint8_t C = (*r8 & 10000000); // store 7th bit for carry
    *r8 = *r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        *r8 |= 00000001; // set bit 0 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        *r8 &= 11111110; // set bit 0 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    
    cpu->_r.f &= ~Z_FLAG;
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

/**
 * @brief shift bits of r8 to the left one and carry bit 7 to carry and carry carry to 0 bit
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void RLr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 10000000); // store 7th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    *r8 = *r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(D == C_FLAG){
        *r8 |= 00000001;
    }else{
        *r8 &= 11111110;
    }
    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the left one and carry bit 7 to carry and carry to bit 0
 * @param cpu pointer to the cpu
 */
void RLHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 10000000); // store 7th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    r8 = r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }

    if(D == C_FLAG){
        r8 |= 00000001;
    }else{
        r8 &= 11111110;
    }

    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of A to the left one and carry bit 7 to carry and carry carry to 0 bit
 * @param cpu pointer to the cpu
 */
void RLA(struct GB_CPU* cpu){
    uint8_t* r8 = &cpu->_r.a;
    uint8_t C = (*r8 & 10000000); // store 7th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    *r8 = *r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(D == C_FLAG){
        *r8 |= 00000001;
    }else{
        *r8 &= 11111110;
    }
    
    cpu->_r.f &= ~Z_FLAG;
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

/**
 * @brief shift bits of r8 to the right one and carry bit 0 to carry and carry carry to 7 bit
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void RRr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 00000001); // store 7th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    *r8 = *r8 >> 1; // shift all bits to the right 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(D == C_FLAG){
        *r8 |= 10000000;
    }else{
        *r8 &= 01111111;
    }
    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the right one and carry bit 0 to carry and carry to bit 7
 * @param cpu pointer to the cpu
 */
void RRHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 00000001); // store 0th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    r8 = r8 >> 1; // shift all bits to the left 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }

    if(D == C_FLAG){
        r8 |= 10000000;
    }else{
        r8 &= 01111111;
    }

    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of A to the right one and carry bit 0 to carry and carry carry to 7 bit
 * @param cpu pointer to the cpu
 */
void RRA(struct GB_CPU* cpu){
    uint8_t* r8 = &cpu->_r.a;
    uint8_t C = (*r8 & 00000001); // store 0th bit for carry
    uint8_t D = (cpu->_r.f & C_FLAG);
    *r8 = *r8 >> 1; // shift all bits to the left 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(D == C_FLAG){
        *r8 |= 10000000;
    }else{
        *r8 &= 01111111;
    }
    
    cpu->_r.f &= ~Z_FLAG;
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

/**
 * @brief shift bits of r8 to the right one and carry bit 0 to bit 7 and carry
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void RRCr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 00000001); // store 7th bit for carry
    *r8 = *r8 >> 1; // shift all bits to the left 1
    if(C == 00000001){ // if 0th bit was 1
        *r8 |= 10000000; // set bit 7 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        *r8 &= 01111111; // set bit 7 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the right one and carry bit 0 to bit 7 and carry
 * @param cpu pointer to the cpu
 */
void RRCHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 00000001); // store 7th bit for carry
    r8 = r8 >> 1; // shift all bits to the left 1
    if(C == 00000001){ // if 0th bit was 1
        r8 |= 10000000; // set bit 7 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        r8 &= 01111111; // set bit 7 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of A to the right one and carry bit 0 to bit 7 and carry
 * @param cpu pointer to the cpu
 */
void RRCA(struct GB_CPU* cpu){
    uint8_t* r8 = &cpu->_r.a;
    uint8_t C = (*r8 & 00000001); // store 7th bit for carry
    *r8 = *r8 >> 1; // shift all bits to the left 1
    if(C == 00000001){ // if 0th bit was 1
        *r8 |= 10000000; // set bit 7 to 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        *r8 &= 01111111; // set bit 7 to 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    
    cpu->_r.f &= ~Z_FLAG;
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

/**
 * @brief shift bits of r8 to the left one and carry bit 7 to carry
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void SLAr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 10000000); // store 7th bit for carry
    *r8 = *r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    *r8 &= 11111110;

    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the left one and carry bit 7 to carry
 * @param cpu pointer to the cpu
 */
void SLAHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 10000000); // store 7th bit for carry
    r8 = r8 << 1; // shift all bits to the left 1
    if(C == 10000000){ // if 7th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 7th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }

    r8 &= 11111110;

    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of r8 to the right one and carry bit 0 to carry
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void SRAr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 00000001); // store 0th bit for carry
    uint8_t D = (*r8 & 10000000); // store 7th bit for carry
    *r8 = *r8 >> 1; // shift all bits to the right 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    
    *r8 |= D;

    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the right one and carry bit 0 to carry
 * @param cpu pointer to the cpu
 */
void SRAHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 00000001); // store 0th bit for carry
    uint8_t D = (r8 & 10000000);
    r8 = r8 >> 1; // shift all bits to the right 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }

    r8 |= D;

    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of r8 to the right one and carry bit 0 to carry
 * @param cpu pointer to the cpu
 * @param r8 pointer to the 8 bit register to be shifted
 */
void SRLr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t C = (*r8 & 00000001); // store 7th bit for carry
    *r8 = *r8 >> 1; // shift all bits to the right 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }
    
    *r8 &= 01111111;

    if(*r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief shift bits of HL to the right one and carry bit 0 to carry
 * @param cpu pointer to the cpu
 */
void SRLHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t C = (r8 & 00000001); // store 0th bit for carry
    r8 = r8 >> 1; // shift all bits to the right 1
    if(C == 00000001){ // if 0th bit was 1
        cpu->_r.f |= C_FLAG; // turn on carry flag
    }else if (C == 00000000){ // if 0th bit was 0
        cpu->_r.f &= ~C_FLAG; // turn off carry flag
    }
    else{
        printf("Somethings gone wrong, somethings gone very very wrong.");
    }

    r8 &= 01111111;

    if(r8 == 0){
       cpu->_r.f |= Z_FLAG; 
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief Swap upper 4 bits and lower 4 bits of r8
 * @param cpu pointer to the cpu
 * @param r8 pointer to 8 bit register to be swapped
 */
void SWAPr8(struct GB_CPU* cpu, uint8_t* r8){
    uint8_t Upp = (*r8 & 11110000);
    uint8_t Low = (*r8 & 00001111);

    Upp = Upp >> 4;
    Low = Low << 4;

    *r8 = (Upp | Low);

    if(*r8 == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~C_FLAG;
    cpu->_r.f &= ~H_FLAG;
    cpu->_r.f &= ~N_FLAG;

    cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc +=2;
}

/**
 * @brief Swap upper 4 bits and lower 4 bits of HL
 * @param cpu pointer to the cpu
 */
void SWAPHL(struct GB_CPU* cpu){
    uint8_t r8 = MMU_RB(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), cpu);
    uint8_t Upp = (r8 & 11110000);
    uint8_t Low = (r8 & 00001111);

    Upp = Upp >> 4;
    Low = Low << 4;

    r8 = (Upp | Low);

    if(r8 == 0){
        cpu->_r.f |= Z_FLAG;
    }else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~C_FLAG;
    cpu->_r.f &= ~H_FLAG;
    cpu->_r.f &= ~N_FLAG;

    MMU_wb(&cpu->mmu, ((cpu->_r.h << 8) + cpu->_r.l), r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 2;
}

#pragma endregion Bit Shift

#pragma region Jumps and Subroutines

#pragma region Calls

/**
 * @brief store the address of the next instruction into the stack and then execute a JP n16.
 * @param cpu pointer to the cpu
 */
void CALLn16(struct GB_CPU* cpu){
    uint16_t store = cpu->_r.pc + 3;
    
    DECsp;
    LD_r8(cpu, cpu->_r.sp,store>>8);
    DECsp;
    LD_r8(cpu, cpu->_r.sp, (uint8_t)store);


    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    cpu->_r.pc = r16; //set pc equal to destination (jump to n16)

    cpu->_r.m = 6; cpu->_r.t = 24; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    
}

/**
 * @brief IF 0 flag is not set, store the address of the next instruction into the stack and then execute a JP n16. Otherwise just go to the next instruction.
 * @param cpu pointer to the cpu
 */
void CALLNZ(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(((tempf &= ~Z_FLAG) ==  cpu->_r.f)){
        uint16_t store = cpu->_r.pc + 3;
        DECsp;
        LD_r8(cpu, cpu->_r.sp,store>>8);
        DECsp;
        LD_r8(cpu, cpu->_r.sp, (uint8_t)store);

        uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)

        cpu->_r.m = 6; cpu->_r.t = 24; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
    
}

/**
 * @brief IF 0 flag is set, store the address of the next instruction into the stack and then execute a JP n16. Otherwise just go to the next instruction.
 * @param cpu pointer to the cpu
 */
void CALLZ(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(!((tempf &= ~Z_FLAG) ==  cpu->_r.f)){
        uint16_t store = cpu->_r.pc + 3;
        DECsp;
        LD_r8(cpu, cpu->_r.sp,store>>8);
        DECsp;
        LD_r8(cpu, cpu->_r.sp, (uint8_t)store);

        uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        
        cpu->_r.m = 6; cpu->_r.t = 24; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
    
}

/**
 * @brief IF Carry flag is not set, store the address of the next instruction into the stack and then execute a JP n16. Otherwise just go to the next instruction.
 * @param cpu pointer to the cpu
 */
void CALLNC(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(((tempf &= ~C_FLAG) ==  cpu->_r.f)){
        uint16_t store = cpu->_r.pc + 3;
        DECsp;
        LD_r8(cpu, cpu->_r.sp,store>>8);
        DECsp;
        LD_r8(cpu, cpu->_r.sp, (uint8_t)store);

        uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)

        cpu->_r.m = 6; cpu->_r.t = 24; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
    
}

/**
 * @brief IF Carry flag is set, store the address of the next instruction into the stack and then execute a JP n16. Otherwise just go to the next instruction.
 * @param cpu pointer to the cpu
 */
void CALLC(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(!((tempf &= ~C_FLAG) ==  cpu->_r.f)){
        uint16_t store = cpu->_r.pc + 3;
        DECsp;
        LD_r8(cpu, cpu->_r.sp,store>>8);
        DECsp;
        LD_r8(cpu, cpu->_r.sp, (uint8_t)store);

        uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        
        cpu->_r.m = 6; cpu->_r.t = 24; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
    
}

/**
 * @brief Make a Call and then jump to vector assigned address AKA: RST vec
 * @param cpu pointer to the cpu
 * @param vec address vector slotted in exec
 */
void RST(struct GB_CPU* cpu, uint8_t vec){
    uint16_t store = cpu->_r.pc + 3;
    
    cpu->_r.sp--;
    LD_r8(cpu, cpu->_r.sp,store>>8);
    cpu->_r.sp--;
    LD_r8(cpu, cpu->_r.sp, (uint8_t)store);

    cpu->_r.pc = vec; //set pc equal to destination (jump to n16)

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}


#pragma endregion Calls

#pragma region JUMP (JP)

/**
 * @brief Jump to address n16 if the 0 flag is not checked
 * @param cpu pointer to CPU
 */
void JPNZ(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        cpu->_r.f = tempf;
        cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
}

/**
 * @brief Jump to address n16 if the Z flag is checked
 * @param cpu pointer to CPU
 */
void JPZ(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        cpu->_r.f = tempf;
        cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
}

/**
 * @brief Jump to address n16 if the Carry flag is not checked
 * @param cpu pointer to CPU
 */
void JPNC(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~C_FLAG) ==  tempf)){
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        cpu->_r.f = tempf;
        cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
}

/**
 * @brief Jump to address n16 if the Carry flag is checked
 * @param cpu pointer to CPU
 */
void JPC(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~C_FLAG) ==  tempf)){
        cpu->_r.pc = r16; //set pc equal to destination (jump to n16)
        cpu->_r.f = tempf;
        cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=3;
    }
}

/**
 * @brief Jump to address n16
 * @param cpu pointer to CPU
 */
void JPn16(struct GB_CPU* cpu){
    uint16_t r16 = ( (MMU_rb(&cpu->mmu, cpu->_r.pc+2, cpu)<<8) +  MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu)); //get n16 value
    cpu->_r.pc = r16; //set pc equal to destination (jump to n16)

    cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Jump to address HL
 * @param cpu pointer to CPU
 */
void JPHL(struct GB_CPU* cpu){
    uint16_t HL = (cpu->_r.h<<8) + cpu->_r.l; // create a variable HL that combines the two 8 bit registers h and l
    cpu->_r.pc = HL; //set pc equal to destination (jump to n16)

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Jump a i8 number of bytes provided by pc+1, perform the jump as an offset from pc+2
 * @param cpu pointer to cpu
 */
void JRn16(struct GB_CPU* cpu){
    int8_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    cpu->_r.pc = (cpu->_r.pc+2) +  r8; // set pointer to the offset from the next instruction 
    cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Relative jump if no zero flag
 * @param cpu pointer to cpu
 */
void JRNZ(struct GB_CPU* cpu){
    int8_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=2;
    }
}

/**
 * @brief Relative jump if zero flag
 * @param cpu pointer to cpu
 */
void JRZ(struct GB_CPU* cpu){
    int8_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=2;
    }
}

/**
 * @brief Relative jump if no carry flag
 * @param cpu pointer to cpu
 */
void JRNC(struct GB_CPU* cpu){
    int8_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~C_FLAG) ==  tempf)){
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=2;
    }
}

/**
 * @brief Relative jump if carry flag
 * @param cpu pointer to cpu
 */
void JRC(struct GB_CPU* cpu){
    int8_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~C_FLAG) ==  tempf)){
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
    else{
        cpu->_r.f = tempf;
        cpu->_r.m = 2; cpu->_r.t = 8; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc+=2;
    }
}

#pragma endregion JUMP (JP)

#pragma region RET

/**
 * @brief return from subroutine. Essentially POP PC 
 * @param cpu pointer to the cpu
 */
void RET(struct GB_CPU* cpu){
    uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
    INCsp;
    cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
    cpu->_r.pc = cpu->_r.pc<<8;
    cpu->_r.pc += temp;
    INCsp;

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief return from subroutine, enable interupts
 * @param cpu pointer to the cpu
 */
void RETI(struct GB_CPU* cpu){
    uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
    cpu->_r.sp++;
    cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
    cpu->_r.pc = cpu->_r.pc<<8;
    cpu->_r.pc += temp;
    cpu->_r.sp++;
    ExecOp(cpu, cpu->_r.pc+1);
    cpu->_r.ime = 1;

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief IF 0 flag is not set, return from subroutine. Essentially POP PC. Otherwise just go next
 * @param cpu pointer to the cpu
 */
void RETNZ(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(((tempf &= ~Z_FLAG) ==  cpu->_r.f)){

        uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
        INCsp;
        cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
        cpu->_r.pc = cpu->_r.pc<<8;
        cpu->_r.pc += temp;
        INCsp;

        cpu->_r.m = 5; cpu->_r.t = 20; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc++;
    }
}

/**
 * @brief IF 0 flag is set, return from subroutine. Essentially POP PC. Otherwise just go next
 * @param cpu pointer to the cpu
 */
void RETZ(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(!((tempf &= ~Z_FLAG) ==  cpu->_r.f)){

        uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
        INCsp;
        cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
        cpu->_r.pc = cpu->_r.pc<<8;
        cpu->_r.pc += temp;
        INCsp;

        cpu->_r.m = 5; cpu->_r.t = 20; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc++;
    }
}

/**
 * @brief IF C flag is not set, return from subroutine. Essentially POP PC. Otherwise just go next
 * @param cpu pointer to the cpu
 */
void RETNC(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(((tempf &= ~C_FLAG) ==  cpu->_r.f)){

        uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
        INCsp;
        cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
        cpu->_r.pc = cpu->_r.pc<<8;
        cpu->_r.pc += temp;
        INCsp;

        cpu->_r.m = 5; cpu->_r.t = 20; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc++;
    }
}

/**
 * @brief IF C flag is set, return from subroutine. Essentially POP PC. Otherwise just go next
 * @param cpu pointer to the cpu
 */
void RETC(struct GB_CPU* cpu){

    uint8_t tempf = cpu->_r.f;
    if(!((tempf &= ~C_FLAG) ==  cpu->_r.f)){

        uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
        INCsp;
        cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
        cpu->_r.pc = cpu->_r.pc<<8;
        cpu->_r.pc += temp;
        INCsp;

        cpu->_r.m = 5; cpu->_r.t = 20; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.m = 2; cpu->_r.t = 8; //Time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        cpu->_r.pc++;
    }
}

void RET(struct GB_CPU* cpu){
    uint8_t temp = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
    INCsp;
    cpu->_r.pc = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
    cpu->_r.pc = cpu->_r.pc<<8;
    cpu->_r.pc += temp;
    INCsp;

    cpu->_r.m = 4; cpu->_r.t = 16; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

#pragma endregion RET

#pragma endregion Jumps and Subroutines

#pragma region Interrupt-related instructions

//started October 2nd, Switch case needs to be finished first. finished OCT 5th

/**
 * @brief Disable interupts by clearing the IME flag
 * @param cpu pointer to the cpu
 */
void DI(struct GB_CPU* cpu){

    cpu->_r.ime = 0; // disables Interupt master enable

    pending_ei = 0;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
} 

/**
 * @brief Enable interupts by setting the IME flag
 * @param cpu pointer to the cpu
 */
void EI(struct GB_CPU* cpu){

    pending_ei = 2;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

#pragma endregion Interrupt-related instructions

#pragma region PushPop


/**
 * @brief pop from the stack into register F then register A.
 * @param cpu pointer to the cpu
 */
void POPAF(struct GB_CPU* cpu){

    cpu->_r.f = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
    INCsp;
    cpu->_r.a = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
    INCsp;

    cpu->_r.f &= 11110000;

    cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief pop sp from the stack into r16.
 * @param cpu pointer to the cpu
 * @param r8 pointer to the low bit
 * @param r82 pointer to the high bit
 */
void POPr16(struct GB_CPU* cpu, uint8_t* r8, uint8_t* r82){

    *r8 = (MMU_rb(&cpu->mmu, cpu->_r.sp, cpu));
    INCsp;
    *r82 = MMU_rb(&cpu->mmu, cpu->_r.sp, cpu);
    INCsp;

    cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief push register AF into the stack 
 * @param cpu pointer to the cpu
 */
void PUSHAF(struct GB_CPU* cpu){

    DECsp;
    LD_r8(cpu, cpu->_r.sp,cpu->_r.a);
    DECsp;
    LD_r8(cpu, cpu->_r.sp, (cpu->_r.f));

    cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief push register r16 into the stack 
 * @param cpu pointer to the cpu
 * @param r8 pointer to low byte
 * @param r82 pointer to high byte
 */
void PUSHr16(struct GB_CPU* cpu, uint8_t* r8, uint8_t* r82){

    DECsp;
    LD_r8(cpu, cpu->_r.sp, *r82);
    DECsp;
    LD_r8(cpu, cpu->_r.sp, *r8);

    cpu->_r.m = 4; cpu->_r.t = 16; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}


#pragma endregion PushPop

#pragma region MISC

/**
 * @brief Function for do nothing.
 */
void NOP(struct GB_CPU* cpu){
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc += 1; //incrememnt past instruction and value
}

/**
 * @brief Decimal Adjust Accumulator
 * @param cpu pointer to cpu
 */
void DAA(struct GB_CPU* cpu){
    uint8_t adj;
    if((cpu->_r.f & N_FLAG) != N_FLAG){
        adj = 0;
        if( (( cpu->_r.f & H_FLAG) == H_FLAG) || (cpu->_r.a & 0xF) > 0x9){
            adj |= 0x6;
        }
        if( ( (cpu->_r.f & C_FLAG) == C_FLAG) || (cpu->_r.a > 0x99)){
            adj |= 0x60;
            cpu->_r.f |= C_FLAG;
        }
        cpu->_r.a += adj;
    }
    else{
        adj = 0;
        if( (( cpu->_r.f & H_FLAG) == H_FLAG)){
             adj |= 0x6;
        }
        if( ( (cpu->_r.f & C_FLAG) == C_FLAG)){
            adj |= 0x60;
        }
        cpu->_r.a -= adj;
    }

    if(cpu->_r.a == 0){
        cpu->_r.f |= Z_FLAG; // zero flag
    }
    else{
        cpu->_r.f &= ~Z_FLAG;
    }

    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++; // move past instruction and value
}

/**
 * @brief Stop operator
 * @param cpu pointer to the cpu
 */
void STOP(struct GB_CPU* cpu){
    stop_var = 0;
    cpu->_r.pc+=2;
}

#pragma endregion MISC / Unsortable

#pragma region Carry Flag Instructions

/**
 * @brief Complement Carry Flag
 * @param cpu pointer to the cpu
 */
void CCF(struct GB_CPU* cpu){

    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    uint8_t tempf = cpu->_r.f;

    // put here a way to inverse c flag (set if not and vice versa)
    tempf &= ~C_FLAG;

    if(tempf == cpu->_r.f){
        cpu->_r.f |= C_FLAG;
    }else{
        cpu->_r.f = tempf;
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

/**
 * @brief Set Carry Flag
 * @param cpu pointer to the cpu
 */
void SCF(struct GB_CPU* cpu){

    cpu->_r.f |= C_FLAG;
    cpu->_r.f &= ~N_FLAG;
    cpu->_r.f &= ~H_FLAG;

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
}

#pragma endregion Carry Flag Instructions

#pragma endregion OPcodes

// Method that actually executes the op codes
void ExecOp(struct GB_CPU* GCPU, uint16_t* pc){
    uint8_t opcode = MMU_rb(&GCPU->mmu, pc, GCPU);
    switch (opcode) {
        case 0x00: NOP(GCPU); break;
        case 0x01: LD_BC(GCPU); break;
        case 0x02: LD_r16A(GCPU, GCPU->_r.b, GCPU->_r.c); break;
        case 0x03: INCr16(GCPU, &GCPU->_r.b, &GCPU->_r.c); break;
        case 0x04: INCr8(GCPU, &GCPU->_r.b); break;
        case 0x05: DECr8(GCPU, &GCPU->_r.b); break;
        case 0x06: LD_nr8(GCPU, &GCPU->_r.b); break;
        case 0x07: RLCA(GCPU); break;
        case 0x08: LD_n16SP(GCPU); break;
        case 0x09: ADDHLBC(GCPU); break;
        case 0x0A: LD_Ar16(GCPU, GCPU->_r.b, GCPU->_r.c); break;
        case 0x0B: DECr16(GCPU, &GCPU->_r.b, &GCPU->_r.c); break;
        case 0x0C: INCr8(GCPU, &GCPU->_r.c); break;
        case 0x0D: DECr8(GCPU, &GCPU->_r.c); break;
        case 0x0E: LD_nr8(GCPU, &GCPU->_r.c); break;
        case 0x0F: RRCA(GCPU); break;
        case 0x10: STOP(GCPU); break;
        case 0x11: LD_DE(GCPU); break;
        case 0x12: LD_r16A(GCPU, GCPU->_r.d, GCPU->_r.e); break;
        case 0x13: INCr16(GCPU, &GCPU->_r.d, &GCPU->_r.e); break;
        case 0x14: INCr8(GCPU, &GCPU->_r.d); break;
        case 0x15: DECr8(GCPU, &GCPU->_r.d); break;
        case 0x16: LD_nr8(GCPU, &GCPU->_r.d); break;
        case 0x17: RLA(GCPU); break;
        case 0x18: JRn16(GCPU); break;
        case 0x19: ADDHLDE(GCPU); break;
        case 0x1A: LD_Ar16(GCPU, GCPU->_r.d, GCPU->_r.e); break;
        case 0x1B: DECr16(GCPU, &GCPU->_r.d, &GCPU->_r.e); break;
        case 0x1C: INCr8(GCPU, &GCPU->_r.e); break;
        case 0x1D: DECr8(GCPU, &GCPU->_r.e); break;
        case 0x1E: LD_nr8(GCPU, &GCPU->_r.e); break;
        case 0x1F: RRA(GCPU); break;
        case 0x20: JRNZ(GCPU); break;
        case 0x21: LD_HL(GCPU); break;
        case 0x22: LD_HLIA(GCPU); break;
        case 0x23: INCr16(GCPU, &GCPU->_r.h, &GCPU->_r.l); break;
        case 0x24: INCr8(GCPU, &GCPU->_r.h); break;
        case 0x25: DECr8(GCPU, &GCPU->_r.h); break;
        case 0x26: LD_nr8(GCPU, &GCPU->_r.h); break;
        case 0x27: DAA(GCPU); break;
        case 0x28: JRZ(GCPU); break;
        case 0x29: ADDHLHL(GCPU); break;
        case 0x2A: LD_AHLI(GCPU); break;
        case 0x2B: DECr16(GCPU, &GCPU->_r.h, &GCPU->_r.l); break;
        case 0x2C: INCr8(GCPU, &GCPU->_r.l); break;
        case 0x2D: DECr8(GCPU, &GCPU->_r.l); break;
        case 0x2E: LD_nr8(GCPU, &GCPU->_r.l); break;
        case 0x2F: CPL(GCPU); break;
        case 0x30: JRNC(GCPU); break;
        case 0x31: LD_SPn16(GCPU); break;
        case 0x32: LD_HLDA(GCPU); break;
        case 0x33: INCsp(GCPU); break;
        case 0x34: INCHL(GCPU); break;
        case 0x35: DECHL(GCPU); break;
        case 0x36: LD_HLn(GCPU); break;
        case 0x37: SCF(GCPU); break;
        case 0x38: JRC(GCPU); break;
        case 0x39: ADDHLSP(GCPU); break;
        case 0x3A: LD_AHLD(GCPU); break;
        case 0x3B: DECsp(GCPU); break;
        case 0x3C: INCr8(GCPU, &GCPU->_r.a); break;
        case 0x3D: DECr8(GCPU, &GCPU->_r.a); break;
        case 0x3E: LD_nr8(GCPU, &GCPU->_r.a); break;
        case 0x3F: CCF(GCPU); break;
        case 0x40: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.b); break;
        case 0x41: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.c); break;
        case 0x42: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.d); break;
        case 0x43: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.e); break;
        case 0x44: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.h); break;
        case 0x45: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.l); break;
        case 0x46: LD_r8HL(GCPU, &GCPU->_r.b); break;
        case 0x47: LD_r8(GCPU, &GCPU->_r.b, GCPU->_r.a); break;
        case 0x48: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.b); break;
        case 0x49: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.c); break;
        case 0x4A: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.d); break;
        case 0x4B: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.e); break;
        case 0x4C: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.h); break;
        case 0x4D: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.l); break;
        case 0x4E: LD_r8HL(GCPU, &GCPU->_r.c); break;
        case 0x4F: LD_r8(GCPU, &GCPU->_r.c, GCPU->_r.a); break;
        case 0x50: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.b); break;
        case 0x51: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.c); break;
        case 0x52: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.d); break;
        case 0x53: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.e); break;
        case 0x54: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.h); break;
        case 0x55: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.l); break;
        case 0x56: LD_r8HL(GCPU, &GCPU->_r.d); break;
        case 0x57: LD_r8(GCPU, &GCPU->_r.d, GCPU->_r.a); break;
        case 0x58: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.b); break;
        case 0x59: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.c); break;
        case 0x5A: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.d); break;
        case 0x5B: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.e); break;
        case 0x5C: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.h); break;
        case 0x5D: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.l); break;
        case 0x5E: LD_r8HL(GCPU, &GCPU->_r.e); break;
        case 0x5F: LD_r8(GCPU, &GCPU->_r.e, GCPU->_r.a); break;
        case 0x60: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.b); break;
        case 0x61: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.c); break;
        case 0x62: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.d); break;
        case 0x63: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.e); break;
        case 0x64: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.h); break;
        case 0x65: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.l); break;
        case 0x66: LD_r8HL(GCPU, &GCPU->_r.h); break;
        case 0x67: LD_r8(GCPU, &GCPU->_r.h, GCPU->_r.a); break;
        case 0x68: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.b); break;
        case 0x69: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.c); break;
        case 0x6A: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.d); break;
        case 0x6B: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.e); break;
        case 0x6C: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.h); break;
        case 0x6D: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.l); break;
        case 0x6E: LD_r8HL(GCPU, &GCPU->_r.l); break;
        case 0x6F: LD_r8(GCPU, &GCPU->_r.l, GCPU->_r.a); break;
        case 0x70: LD_HLr8(GCPU, GCPU->_r.b); break;
        case 0x71: LD_HLr8(GCPU, GCPU->_r.c); break;
        case 0x72: LD_HLr8(GCPU, GCPU->_r.d); break;
        case 0x73: LD_HLr8(GCPU, GCPU->_r.e); break;
        case 0x74: LD_HLr8(GCPU, GCPU->_r.h); break;
        case 0x75: LD_HLr8(GCPU, GCPU->_r.l); break;
            case 0x76: /* HALT(GCPU); // Missing function */ break;
        case 0x77: LD_HLr8(GCPU, GCPU->_r.a); break;
        case 0x78: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.b); break;
        case 0x79: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.c); break;
        case 0x7A: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.d); break;
        case 0x7B: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.e); break;
        case 0x7C: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.h); break;
        case 0x7D: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.l); break;
        case 0x7E: LD_r8HL(GCPU, &GCPU->_r.a); break;
        case 0x7F: LD_r8(GCPU, &GCPU->_r.a, GCPU->_r.a); break;
        case 0x80: ADD_ar8(GCPU, GCPU->_r.b); break;
        case 0x81: ADD_ar8(GCPU, GCPU->_r.c); break;
        case 0x82: ADD_ar8(GCPU, GCPU->_r.d); break;
        case 0x83: ADD_ar8(GCPU, GCPU->_r.e); break;
        case 0x84: ADD_ar8(GCPU, GCPU->_r.h); break;
        case 0x85: ADD_ar8(GCPU, GCPU->_r.l); break;
        case 0x86: ADD_HL(GCPU); break;
        case 0x87: ADD_ar8(GCPU, GCPU->_r.a); break;
        case 0x88: ADC_ar8(GCPU, GCPU->_r.b); break;
        case 0x89: ADC_ar8(GCPU, GCPU->_r.c); break;
        case 0x8A: ADC_ar8(GCPU, GCPU->_r.d); break;
        case 0x8B: ADC_ar8(GCPU, GCPU->_r.e); break;
        case 0x8C: ADC_ar8(GCPU, GCPU->_r.h); break;
        case 0x8D: ADC_ar8(GCPU, GCPU->_r.l); break;
        case 0x8E: ADC_hl(GCPU); break;
        case 0x8F: ADC_ar8(GCPU, GCPU->_r.a); break;
        case 0x90: SUB_ar8(GCPU, GCPU->_r.b); break;
        case 0x91: SUB_ar8(GCPU, GCPU->_r.c); break;
        case 0x92: SUB_ar8(GCPU, GCPU->_r.d); break;
        case 0x93: SUB_ar8(GCPU, GCPU->_r.e); break;
        case 0x94: SUB_ar8(GCPU, GCPU->_r.h); break;
        case 0x95: SUB_ar8(GCPU, GCPU->_r.l); break;
        case 0x96: SUB_HL(GCPU); break;
        case 0x97: SUB_ar8(GCPU, GCPU->_r.a); break;
        case 0x98: SBC_ar8(GCPU, GCPU->_r.b); break;
        case 0x99: SBC_ar8(GCPU, GCPU->_r.c); break;
        case 0x9A: SBC_ar8(GCPU, GCPU->_r.d); break;
        case 0x9B: SBC_ar8(GCPU, GCPU->_r.e); break;
        case 0x9C: SBC_ar8(GCPU, GCPU->_r.h); break;
        case 0x9D: SBC_ar8(GCPU, GCPU->_r.l); break;
        case 0x9E: SBC_hl(GCPU); break;
        case 0x9F: SBC_ar8(GCPU, GCPU->_r.a); break;
        case 0xA0: AND_ar8(GCPU, GCPU->_r.b); break;
        case 0xA1: AND_ar8(GCPU, GCPU->_r.c); break;
        case 0xA2: AND_ar8(GCPU, GCPU->_r.d); break;
        case 0xA3: AND_ar8(GCPU, GCPU->_r.e); break;
        case 0xA4: AND_ar8(GCPU, GCPU->_r.h); break;
        case 0xA5: AND_ar8(GCPU, GCPU->_r.l); break;
        case 0xA6: AND_HL(GCPU); break;
        case 0xA7: AND_ar8(GCPU, GCPU->_r.a); break;
        case 0xA8: XORr8(GCPU, &GCPU->_r.b); break;
        case 0xA9: XORr8(GCPU, &GCPU->_r.c); break;
        case 0xAA: XORr8(GCPU, &GCPU->_r.d); break;
        case 0xAB: XORr8(GCPU, &GCPU->_r.e); break;
        case 0xAC: XORr8(GCPU, &GCPU->_r.h); break;
        case 0xAD: XORr8(GCPU, &GCPU->_r.l); break;
        case 0xAE: XORHL(GCPU); break;
        case 0xAF: XORr8(GCPU, &GCPU->_r.a); break;
        case 0xB0: ORr8(GCPU, &GCPU->_r.b); break;
        case 0xB1: ORr8(GCPU, &GCPU->_r.c); break;
        case 0xB2: ORr8(GCPU, &GCPU->_r.d); break;
        case 0xB3: ORr8(GCPU, &GCPU->_r.e); break;
        case 0xB4: ORr8(GCPU, &GCPU->_r.h); break;
        case 0xB5: ORr8(GCPU, &GCPU->_r.l); break;
        case 0xB6: ORHL(GCPU); break;
        case 0xB7: ORr8(GCPU, &GCPU->_r.a); break;
        case 0xB8: CP_ar8(GCPU, GCPU->_r.b); break;
        case 0xB9: CP_ar8(GCPU, GCPU->_r.c); break;
        case 0xBA: CP_ar8(GCPU, GCPU->_r.d); break;
        case 0xBB: CP_ar8(GCPU, GCPU->_r.e); break;
        case 0xBC: CP_ar8(GCPU, GCPU->_r.h); break;
        case 0xBD: CP_ar8(GCPU, GCPU->_r.l); break;
        case 0xBE: CP_HL(GCPU); break;
        case 0xBF: CP_ar8(GCPU, GCPU->_r.a); break;
        case 0xC0: RETNZ(GCPU); break;
        case 0xC1: POPr16(GCPU, &GCPU->_r.c, &GCPU->_r.b); break;
        case 0xC2: JPNZ(GCPU); break;
        case 0xC3: JPn16(GCPU); break;
        case 0xC4: CALLNZ(GCPU); break;
        case 0xC5: PUSHr16(GCPU, &GCPU->_r.c, &GCPU->_r.b); break;
        case 0xC6: ADD_n2a(GCPU); break;
        case 0xC7: RST(GCPU, 0x00); break;
        case 0xC8: RETZ(GCPU); break;
        case 0xC9: RET(GCPU); break;
        case 0xCA: JPZ(GCPU); break;
        case 0xCB: ExecCbOp(GCPU, &GCPU->_r.pc+1); break;
        case 0xCC: CALLZ(GCPU); break;
        case 0xCD: CALLn16(GCPU); break;
        case 0xCE: ADC_an8(GCPU); break;
        case 0xCF: RST(GCPU, 0x08); break;
        case 0xD0: RETNC(GCPU); break;
        case 0xD1: POPr16(GCPU, &GCPU->_r.e, &GCPU->_r.d); break;
        case 0xD2: JPNC(GCPU); break;
        case 0xD4: CALLNC(GCPU); break;
        case 0xD5: PUSHr16(GCPU, &GCPU->_r.e, &GCPU->_r.d); break;
        case 0xD6: SUB_an(GCPU); break;
        case 0xD7: RST (GCPU, 0x10); break;
        case 0xD8: RETC(GCPU); break;
        case 0xD9: RETI(GCPU); break;
        case 0xDA: JPC(GCPU); break;
        case 0xDC: CALLC(GCPU); break;
        case 0xDE: SBC_an8(GCPU); break;
        case 0xDF: RST(GCPU, 0x18); break;
        case 0xE0: LDH_n16A(GCPU); break;
        case 0xE1: POPr16(GCPU, &GCPU->_r.l, &GCPU->_r.h); break;
        case 0xE2: LDHC(GCPU); break;
        case 0xE5: PUSHr16(GCPU, &GCPU->_r.l, &GCPU->_r.h); break;
        case 0xE6: AND_an(GCPU); break;
        case 0xE7: RST(GCPU, 0x20); break;
        case 0xE8: ADD_SP2d(GCPU); break;
        case 0xE9: JPHL(GCPU); break;
        case 0xEA: LD_n16A(GCPU); break;
        case 0xEE: XORn8(GCPU); break;
        case 0xEF: RST(GCPU,0x28); break;
        case 0xF0: LDHCu8(GCPU); break;
        case 0xF1: POPAF(GCPU); break;
        case 0xF2: LDHAC(GCPU); break;
        case 0xF3: DI(GCPU); break;
        case 0xF5: PUSHAF(GCPU); break;
        case 0xF6: ORn8(GCPU); break;
        case 0xF7: RST(GCPU,0x30); break;
        case 0xF8: LD_HLspe8(GCPU); break;
        case 0xF9: LD_HLSP(GCPU); break;
        case 0xFA: LDH_An16(GCPU); break;
        case 0xFB: EI(GCPU); break;
        case 0xFE: CP_an(GCPU); break;
        case 0xFF: RST(GCPU,0x38); break;
        default: break;
    }
}


void ExecCbOp(struct GB_CPU* GCPU, uint16_t* pc) {
    // Read the opcode that follows the 0xCB prefix
    uint8_t opcode = MMU_rb(&GCPU->mmu, pc, GCPU);

    switch (opcode) {

        // RLC
        case 0x00: RLCr8(GCPU, &GCPU->_r.b); break;
        case 0x01: RLCr8(GCPU, &GCPU->_r.c); break;
        case 0x02: RLCr8(GCPU, &GCPU->_r.d); break;
        case 0x03: RLCr8(GCPU, &GCPU->_r.e); break;
        case 0x04: RLCr8(GCPU, &GCPU->_r.h); break;
        case 0x05: RLCr8(GCPU, &GCPU->_r.l); break;
        case 0x06: RLCHL(GCPU); break;
        case 0x07: RLCr8(GCPU, &GCPU->_r.a); break;

        // RRC
        case 0x08: RRCr8(GCPU, &GCPU->_r.b); break;
        case 0x09: RRCr8(GCPU, &GCPU->_r.c); break;
        case 0x0A: RRCr8(GCPU, &GCPU->_r.d); break;
        case 0x0B: RRCr8(GCPU, &GCPU->_r.e); break;
        case 0x0C: RRCr8(GCPU, &GCPU->_r.h); break;
        case 0x0D: RRCr8(GCPU, &GCPU->_r.l); break;
        case 0x0E: RRCHL(GCPU); break;
        case 0x0F: RRCr8(GCPU, &GCPU->_r.a); break;

        // RL
        case 0x10: RLr8(GCPU, &GCPU->_r.b); break;
        case 0x11: RLr8(GCPU, &GCPU->_r.c); break;
        case 0x12: RLr8(GCPU, &GCPU->_r.d); break;
        case 0x13: RLr8(GCPU, &GCPU->_r.e); break;
        case 0x14: RLr8(GCPU, &GCPU->_r.h); break;
        case 0x15: RLr8(GCPU, &GCPU->_r.l); break;
        case 0x16: RLHL(GCPU); break;
        case 0x17: RLr8(GCPU, &GCPU->_r.a); break;

        // RR
        case 0x18: RRr8(GCPU, &GCPU->_r.b); break;
        case 0x19: RRr8(GCPU, &GCPU->_r.c); break;
        case 0x1A: RRr8(GCPU, &GCPU->_r.d); break;
        case 0x1B: RRr8(GCPU, &GCPU->_r.e); break;
        case 0x1C: RRr8(GCPU, &GCPU->_r.h); break;
        case 0x1D: RRr8(GCPU, &GCPU->_r.l); break;
        case 0x1E: RRHL(GCPU); break;
        case 0x1F: RRr8(GCPU, &GCPU->_r.a); break;

        // SLA
        case 0x20: SLAr8(GCPU, &GCPU->_r.b); break;
        case 0x21: SLAr8(GCPU, &GCPU->_r.c); break;
        case 0x22: SLAr8(GCPU, &GCPU->_r.d); break;
        case 0x23: SLAr8(GCPU, &GCPU->_r.e); break;
        case 0x24: SLAr8(GCPU, &GCPU->_r.h); break;
        case 0x25: SLAr8(GCPU, &GCPU->_r.l); break;
        case 0x26: SLAHL(GCPU); break;
        case 0x27: SLAr8(GCPU, &GCPU->_r.a); break;

        // SRA
        case 0x28: SRAr8(GCPU, &GCPU->_r.b); break;
        case 0x29: SRAr8(GCPU, &GCPU->_r.c); break;
        case 0x2A: SRAr8(GCPU, &GCPU->_r.d); break;
        case 0x2B: SRAr8(GCPU, &GCPU->_r.e); break;
        case 0x2C: SRAr8(GCPU, &GCPU->_r.h); break;
        case 0x2D: SRAr8(GCPU, &GCPU->_r.l); break;
        case 0x2E: SRAHL(GCPU); break;
        case 0x2F: SRAr8(GCPU, &GCPU->_r.a); break;

        // SWAP
        case 0x30: SWAPr8(GCPU, &GCPU->_r.b); break;
        case 0x31: SWAPr8(GCPU, &GCPU->_r.c); break;
        case 0x32: SWAPr8(GCPU, &GCPU->_r.d); break;
        case 0x33: SWAPr8(GCPU, &GCPU->_r.e); break;
        case 0x34: SWAPr8(GCPU, &GCPU->_r.h); break;
        case 0x35: SWAPr8(GCPU, &GCPU->_r.l); break;
        case 0x36: SWAPHL(GCPU); break;
        case 0x37: SWAPr8(GCPU, &GCPU->_r.a); break;

        // SRL
        case 0x38: SRLr8(GCPU, &GCPU->_r.b); break;
        case 0x39: SRLr8(GCPU, &GCPU->_r.c); break;
        case 0x3A: SRLr8(GCPU, &GCPU->_r.d); break;
        case 0x3B: SRLr8(GCPU, &GCPU->_r.e); break;
        case 0x3C: SRLr8(GCPU, &GCPU->_r.h); break;
        case 0x3D: SRLr8(GCPU, &GCPU->_r.l); break;
        case 0x3E: SRLHL(GCPU); break;
        case 0x3F: SRLr8(GCPU, &GCPU->_r.a); break;

        // BIT
        case 0x40: BIT_u3r8(GCPU, 0, &GCPU->_r.b); break;
        case 0x41: BIT_u3r8(GCPU, 0, &GCPU->_r.c); break;
        case 0x42: BIT_u3r8(GCPU, 0, &GCPU->_r.d); break;
        case 0x43: BIT_u3r8(GCPU, 0, &GCPU->_r.e); break;
        case 0x44: BIT_u3r8(GCPU, 0, &GCPU->_r.h); break;
        case 0x45: BIT_u3r8(GCPU, 0, &GCPU->_r.l); break;
        case 0x46: BIT_u3HL(GCPU, 0); break;
        case 0x47: BIT_u3r8(GCPU, 0, &GCPU->_r.a); break;

        case 0x48: BIT_u3r8(GCPU, 1, &GCPU->_r.b); break;
        case 0x49: BIT_u3r8(GCPU, 1, &GCPU->_r.c); break;
        case 0x4A: BIT_u3r8(GCPU, 1, &GCPU->_r.d); break;
        case 0x4B: BIT_u3r8(GCPU, 1, &GCPU->_r.e); break;
        case 0x4C: BIT_u3r8(GCPU, 1, &GCPU->_r.h); break;
        case 0x4D: BIT_u3r8(GCPU, 1, &GCPU->_r.l); break;
        case 0x4E: BIT_u3HL(GCPU, 1); break;
        case 0x4F: BIT_u3r8(GCPU, 1, &GCPU->_r.a); break;

        case 0x50: BIT_u3r8(GCPU, 2, &GCPU->_r.b); break;
        case 0x51: BIT_u3r8(GCPU, 2, &GCPU->_r.c); break;
        case 0x52: BIT_u3r8(GCPU, 2, &GCPU->_r.d); break;
        case 0x53: BIT_u3r8(GCPU, 2, &GCPU->_r.e); break;
        case 0x54: BIT_u3r8(GCPU, 2, &GCPU->_r.h); break;
        case 0x55: BIT_u3r8(GCPU, 2, &GCPU->_r.l); break;
        case 0x56: BIT_u3HL(GCPU, 2); break;
        case 0x57: BIT_u3r8(GCPU, 2, &GCPU->_r.a); break;

        case 0x58: BIT_u3r8(GCPU, 3, &GCPU->_r.b); break;
        case 0x59: BIT_u3r8(GCPU, 3, &GCPU->_r.c); break;
        case 0x5A: BIT_u3r8(GCPU, 3, &GCPU->_r.d); break;
        case 0x5B: BIT_u3r8(GCPU, 3, &GCPU->_r.e); break;
        case 0x5C: BIT_u3r8(GCPU, 3, &GCPU->_r.h); break;
        case 0x5D: BIT_u3r8(GCPU, 3, &GCPU->_r.l); break;
        case 0x5E: BIT_u3HL(GCPU, 3); break;
        case 0x5F: BIT_u3r8(GCPU, 3, &GCPU->_r.a); break;

        case 0x60: BIT_u3r8(GCPU, 4, &GCPU->_r.b); break;
        case 0x61: BIT_u3r8(GCPU, 4, &GCPU->_r.c); break;
        case 0x62: BIT_u3r8(GCPU, 4, &GCPU->_r.d); break;
        case 0x63: BIT_u3r8(GCPU, 4, &GCPU->_r.e); break;
        case 0x64: BIT_u3r8(GCPU, 4, &GCPU->_r.h); break;
        case 0x65: BIT_u3r8(GCPU, 4, &GCPU->_r.l); break;
        case 0x66: BIT_u3HL(GCPU, 4); break;
        case 0x67: BIT_u3r8(GCPU, 4, &GCPU->_r.a); break;

        case 0x68: BIT_u3r8(GCPU, 5, &GCPU->_r.b); break;
        case 0x69: BIT_u3r8(GCPU, 5, &GCPU->_r.c); break;
        case 0x6A: BIT_u3r8(GCPU, 5, &GCPU->_r.d); break;
        case 0x6B: BIT_u3r8(GCPU, 5, &GCPU->_r.e); break;
        case 0x6C: BIT_u3r8(GCPU, 5, &GCPU->_r.h); break;
        case 0x6D: BIT_u3r8(GCPU, 5, &GCPU->_r.l); break;
        case 0x6E: BIT_u3HL(GCPU, 5); break;
        case 0x6F: BIT_u3r8(GCPU, 5, &GCPU->_r.a); break;

        case 0x70: BIT_u3r8(GCPU, 6, &GCPU->_r.b); break;
        case 0x71: BIT_u3r8(GCPU, 6, &GCPU->_r.c); break;
        case 0x72: BIT_u3r8(GCPU, 6, &GCPU->_r.d); break;
        case 0x73: BIT_u3r8(GCPU, 6, &GCPU->_r.e); break;
        case 0x74: BIT_u3r8(GCPU, 6, &GCPU->_r.h); break;
        case 0x75: BIT_u3r8(GCPU, 6, &GCPU->_r.l); break;
        case 0x76: BIT_u3HL(GCPU, 6); break;
        case 0x77: BIT_u3r8(GCPU, 6, &GCPU->_r.a); break;

        case 0x78: BIT_u3r8(GCPU, 7, &GCPU->_r.b); break;
        case 0x79: BIT_u3r8(GCPU, 7, &GCPU->_r.c); break;
        case 0x7A: BIT_u3r8(GCPU, 7, &GCPU->_r.d); break;
        case 0x7B: BIT_u3r8(GCPU, 7, &GCPU->_r.e); break;
        case 0x7C: BIT_u3r8(GCPU, 7, &GCPU->_r.h); break;
        case 0x7D: BIT_u3r8(GCPU, 7, &GCPU->_r.l); break;
        case 0x7E: BIT_u3HL(GCPU, 7); break;
        case 0x7F: BIT_u3r8(GCPU, 7, &GCPU->_r.a); break;

        // RES
        case 0x80: RES_u3r8(GCPU, 0, &GCPU->_r.b); break;
        case 0x81: RES_u3r8(GCPU, 0, &GCPU->_r.c); break;
        case 0x82: RES_u3r8(GCPU, 0, &GCPU->_r.d); break;
        case 0x83: RES_u3r8(GCPU, 0, &GCPU->_r.e); break;
        case 0x84: RES_u3r8(GCPU, 0, &GCPU->_r.h); break;
        case 0x85: RES_u3r8(GCPU, 0, &GCPU->_r.l); break;
        case 0x86: RES_u3HL(GCPU, 0); break;
        case 0x87: RES_u3r8(GCPU, 0, &GCPU->_r.a); break;

        case 0x88: RES_u3r8(GCPU, 1, &GCPU->_r.b); break;
        case 0x89: RES_u3r8(GCPU, 1, &GCPU->_r.c); break;
        case 0x8A: RES_u3r8(GCPU, 1, &GCPU->_r.d); break;
        case 0x8B: RES_u3r8(GCPU, 1, &GCPU->_r.e); break;
        case 0x8C: RES_u3r8(GCPU, 1, &GCPU->_r.h); break;
        case 0x8D: RES_u3r8(GCPU, 1, &GCPU->_r.l); break;
        case 0x8E: RES_u3HL(GCPU, 1); break;
        case 0x8F: RES_u3r8(GCPU, 1, &GCPU->_r.a); break;

        case 0x90: RES_u3r8(GCPU, 2, &GCPU->_r.b); break;
        case 0x91: RES_u3r8(GCPU, 2, &GCPU->_r.c); break;
        case 0x92: RES_u3r8(GCPU, 2, &GCPU->_r.d); break;
        case 0x93: RES_u3r8(GCPU, 2, &GCPU->_r.e); break;
        case 0x94: RES_u3r8(GCPU, 2, &GCPU->_r.h); break;
        case 0x95: RES_u3r8(GCPU, 2, &GCPU->_r.l); break;
        case 0x96: RES_u3HL(GCPU, 2); break;
        case 0x97: RES_u3r8(GCPU, 2, &GCPU->_r.a); break;

        case 0x98: RES_u3r8(GCPU, 3, &GCPU->_r.b); break;
        case 0x99: RES_u3r8(GCPU, 3, &GCPU->_r.c); break;
        case 0x9A: RES_u3r8(GCPU, 3, &GCPU->_r.d); break;
        case 0x9B: RES_u3r8(GCPU, 3, &GCPU->_r.e); break;
        case 0x9C: RES_u3r8(GCPU, 3, &GCPU->_r.h); break;
        case 0x9D: RES_u3r8(GCPU, 3, &GCPU->_r.l); break;
        case 0x9E: RES_u3HL(GCPU, 3); break;
        case 0x9F: RES_u3r8(GCPU, 3, &GCPU->_r.a); break;

        case 0xA0: RES_u3r8(GCPU, 4, &GCPU->_r.b); break;
        case 0xA1: RES_u3r8(GCPU, 4, &GCPU->_r.c); break;
        case 0xA2: RES_u3r8(GCPU, 4, &GCPU->_r.d); break;
        case 0xA3: RES_u3r8(GCPU, 4, &GCPU->_r.e); break;
        case 0xA4: RES_u3r8(GCPU, 4, &GCPU->_r.h); break;
        case 0xA5: RES_u3r8(GCPU, 4, &GCPU->_r.l); break;
        case 0xA6: RES_u3HL(GCPU, 4); break;
        case 0xA7: RES_u3r8(GCPU, 4, &GCPU->_r.a); break;

        case 0xA8: RES_u3r8(GCPU, 5, &GCPU->_r.b); break;
        case 0xA9: RES_u3r8(GCPU, 5, &GCPU->_r.c); break;
        case 0xAA: RES_u3r8(GCPU, 5, &GCPU->_r.d); break;
        case 0xAB: RES_u3r8(GCPU, 5, &GCPU->_r.e); break;
        case 0xAC: RES_u3r8(GCPU, 5, &GCPU->_r.h); break;
        case 0xAD: RES_u3r8(GCPU, 5, &GCPU->_r.l); break;
        case 0xAE: RES_u3HL(GCPU, 5); break;
        case 0xAF: RES_u3r8(GCPU, 5, &GCPU->_r.a); break;

        case 0xB0: RES_u3r8(GCPU, 6, &GCPU->_r.b); break;
        case 0xB1: RES_u3r8(GCPU, 6, &GCPU->_r.c); break;
        case 0xB2: RES_u3r8(GCPU, 6, &GCPU->_r.d); break;
        case 0xB3: RES_u3r8(GCPU, 6, &GCPU->_r.e); break;
        case 0xB4: RES_u3r8(GCPU, 6, &GCPU->_r.h); break;
        case 0xB5: RES_u3r8(GCPU, 6, &GCPU->_r.l); break;
        case 0xB6: RES_u3HL(GCPU, 6); break;
        case 0xB7: RES_u3r8(GCPU, 6, &GCPU->_r.a); break;

        case 0xB8: RES_u3r8(GCPU, 7, &GCPU->_r.b); break;
        case 0xB9: RES_u3r8(GCPU, 7, &GCPU->_r.c); break;
        case 0xBA: RES_u3r8(GCPU, 7, &GCPU->_r.d); break;
        case 0xBB: RES_u3r8(GCPU, 7, &GCPU->_r.e); break;
        case 0xBC: RES_u3r8(GCPU, 7, &GCPU->_r.h); break;
        case 0xBD: RES_u3r8(GCPU, 7, &GCPU->_r.l); break;
        case 0xBE: RES_u3HL(GCPU, 7); break;
        case 0xBF: RES_u3r8(GCPU, 7, &GCPU->_r.a); break;
        
        // SET
        case 0xC0: SET_u3r8(GCPU, 0, &GCPU->_r.b); break;
        case 0xC1: SET_u3r8(GCPU, 0, &GCPU->_r.c); break;
        case 0xC2: SET_u3r8(GCPU, 0, &GCPU->_r.d); break;
        case 0xC3: SET_u3r8(GCPU, 0, &GCPU->_r.e); break;
        case 0xC4: SET_u3r8(GCPU, 0, &GCPU->_r.h); break;
        case 0xC5: SET_u3r8(GCPU, 0, &GCPU->_r.l); break;
        case 0xC6: SET_u3HL(GCPU, 0); break;
        case 0xC7: SET_u3r8(GCPU, 0, &GCPU->_r.a); break;

        case 0xC8: SET_u3r8(GCPU, 1, &GCPU->_r.b); break;
        case 0xC9: SET_u3r8(GCPU, 1, &GCPU->_r.c); break;
        case 0xCA: SET_u3r8(GCPU, 1, &GCPU->_r.d); break;
        case 0xCB: SET_u3r8(GCPU, 1, &GCPU->_r.e); break;
        case 0xCC: SET_u3r8(GCPU, 1, &GCPU->_r.h); break;
        case 0xCD: SET_u3r8(GCPU, 1, &GCPU->_r.l); break;
        case 0xCE: SET_u3HL(GCPU, 1); break;
        case 0xCF: SET_u3r8(GCPU, 1, &GCPU->_r.a); break;

        case 0xD0: SET_u3r8(GCPU, 2, &GCPU->_r.b); break;
        case 0xD1: SET_u3r8(GCPU, 2, &GCPU->_r.c); break;
        case 0xD2: SET_u3r8(GCPU, 2, &GCPU->_r.d); break;
        case 0xD3: SET_u3r8(GCPU, 2, &GCPU->_r.e); break;
        case 0xD4: SET_u3r8(GCPU, 2, &GCPU->_r.h); break;
        case 0xD5: SET_u3r8(GCPU, 2, &GCPU->_r.l); break;
        case 0xD6: SET_u3HL(GCPU, 2); break;
        case 0xD7: SET_u3r8(GCPU, 2, &GCPU->_r.a); break;

        case 0xD8: SET_u3r8(GCPU, 3, &GCPU->_r.b); break;
        case 0xD9: SET_u3r8(GCPU, 3, &GCPU->_r.c); break;
        case 0xDA: SET_u3r8(GCPU, 3, &GCPU->_r.d); break;
        case 0xDB: SET_u3r8(GCPU, 3, &GCPU->_r.e); break;
        case 0xDC: SET_u3r8(GCPU, 3, &GCPU->_r.h); break;
        case 0xDD: SET_u3r8(GCPU, 3, &GCPU->_r.l); break;
        case 0xDE: SET_u3HL(GCPU, 3); break;
        case 0xDF: SET_u3r8(GCPU, 3, &GCPU->_r.a); break;

        case 0xE0: SET_u3r8(GCPU, 4, &GCPU->_r.b); break;
        case 0xE1: SET_u3r8(GCPU, 4, &GCPU->_r.c); break;
        case 0xE2: SET_u3r8(GCPU, 4, &GCPU->_r.d); break;
        case 0xE3: SET_u3r8(GCPU, 4, &GCPU->_r.e); break;
        case 0xE4: SET_u3r8(GCPU, 4, &GCPU->_r.h); break;
        case 0xE5: SET_u3r8(GCPU, 4, &GCPU->_r.l); break;
        case 0xE6: SET_u3HL(GCPU, 4); break;
        case 0xE7: SET_u3r8(GCPU, 4, &GCPU->_r.a); break;

        case 0xE8: SET_u3r8(GCPU, 5, &GCPU->_r.b); break;
        case 0xE9: SET_u3r8(GCPU, 5, &GCPU->_r.c); break;
        case 0xEA: SET_u3r8(GCPU, 5, &GCPU->_r.d); break;
        case 0xEB: SET_u3r8(GCPU, 5, &GCPU->_r.e); break;
        case 0xEC: SET_u3r8(GCPU, 5, &GCPU->_r.h); break;
        case 0xED: SET_u3r8(GCPU, 5, &GCPU->_r.l); break;
        case 0xEE: SET_u3HL(GCPU, 5); break;
        case 0xEF: SET_u3r8(GCPU, 5, &GCPU->_r.a); break;

        case 0xF0: SET_u3r8(GCPU, 6, &GCPU->_r.b); break;
        case 0xF1: SET_u3r8(GCPU, 6, &GCPU->_r.c); break;
        case 0xF2: SET_u3r8(GCPU, 6, &GCPU->_r.d); break;
        case 0xF3: SET_u3r8(GCPU, 6, &GCPU->_r.e); break;
        case 0xF4: SET_u3r8(GCPU, 6, &GCPU->_r.h); break;
        case 0xF5: SET_u3r8(GCPU, 6, &GCPU->_r.l); break;
        case 0xF6: SET_u3HL(GCPU, 6); break;
        case 0xF7: SET_u3r8(GCPU, 6, &GCPU->_r.a); break;

        case 0xF8: SET_u3r8(GCPU, 7, &GCPU->_r.b); break;
        case 0xF9: SET_u3r8(GCPU, 7, &GCPU->_r.c); break;
        case 0xFA: SET_u3r8(GCPU, 7, &GCPU->_r.d); break;
        case 0xFB: SET_u3r8(GCPU, 7, &GCPU->_r.e); break;
        case 0xFC: SET_u3r8(GCPU, 7, &GCPU->_r.h); break;
        case 0xFD: SET_u3r8(GCPU, 7, &GCPU->_r.l); break;
        case 0xFE: SET_u3HL(GCPU, 7); break;
        case 0xFF: SET_u3r8(GCPU, 7, &GCPU->_r.a); break;

        default:
            break;
    }
}

void main(){

    initialize();
    pending_ei = 0;
    stop_var = 1;

    while(stop_var){

        ExecOp(&GlobalCPU, GlobalCPU._r.pc);

        if (pending_ei) {
            pending_ei--;

            if (!pending_ei) {
                GlobalCPU._r.ime = 1;
        }
    }

    }



}

