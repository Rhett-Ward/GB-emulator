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

struct GB_CPU GCPU;

void initialize(){
    GCPU._r.a = 0;
    GCPU._r.b = 0;
    GCPU._r.c = 0;
    GCPU._r.d = 0;
    GCPU._r.e = 0;
    GCPU._r.f = 0;
    GCPU._r.h = 0;
    GCPU._r.l = 0;

    GCPU._r.pc = 0;
    GCPU._r.sp = 0;
    GCPU._r.ime = 0;

    GCPU._c.m = 0;
    GCPU._c.t = 0;
}

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
 * @brief Copy the value at $FF00 + C into register A AKA:LDH(C,A)
 * @param cpu Pointer to the cpu
 */
void LDHC (struct GB_CPU* cpu){
    MMU_wb(&cpu->mmu, cpu->_r.c + 0xFF00, cpu->_r.a);

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
 * @brief jump a number of bytes provided by the next address in memory
 * @param cpu pointer to cpu
 */
void JRn16(struct GB_CPU* cpu){
    uint16_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    if(r8 > 127){
        r8 = (r8-256);
        cpu->_r.pc = (cpu->_r.pc+2) + r8;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
    else{
        cpu->_r.pc = (cpu->_r.pc+2) +  r8;
        cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
        cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
        return;
    }
}

/**
 * @brief Relative jump if no zero flag
 * @param cpu pointer to cpu
 */
void JRNZ(struct GB_CPU* cpu){
    uint16_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
        if(r8 > 127){
            cpu->_r.f = tempf;
            r8 = (r8-256);
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
        }
        else{
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) +  r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
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
    uint16_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~Z_FLAG) ==  tempf)){
        if(r8 > 127){
            cpu->_r.f = tempf;
            r8 = (r8-256);
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
        }
        else{
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) +  r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
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
    uint16_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(((cpu->_r.f &= ~C_FLAG) ==  tempf)){
        if(r8 > 127){
            cpu->_r.f = tempf;
            r8 = (r8-256);
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
        }
        else{
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) +  r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
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
    uint16_t r8 = MMU_rb(&cpu->mmu, cpu->_r.pc+1, cpu); //get jump value
    uint8_t tempf = cpu->_r.f;
    if(!((cpu->_r.f &= ~C_FLAG) ==  tempf)){
        if(r8 > 127){
            cpu->_r.f = tempf;
            r8 = (r8-256);
            cpu->_r.pc = (cpu->_r.pc+2) + r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
        }
        else{
            cpu->_r.f = tempf;
            cpu->_r.pc = (cpu->_r.pc+2) +  r8;
            cpu->_r.m = 3; cpu->_r.t = 12; //time of last cycle
            cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
            return;
    }
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

#pragma endregion RET

#pragma endregion Jumps and Subroutines

#pragma region Interrupt-related instructions

//started October 2nd, Switch case needs to be finished first.

/**
 * @brief Disable interupts by clearing the IME flag
 * @param cpu pointer to the cpu
 */
/* void DI(struct GB_CPU* cpu){
    cpu->_r.ime = 0;
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
} */

/**
 * @brief Enable interupts by setting the IME flag
 * @param cpu pointer to the cpu
 */
/* void EI(struct GB_CPU* cpu){
    
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    cpu->_r.pc++;
} */

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

#pragma region MISC / Unsortable

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



#pragma endregion MISC / Unsortable

/**
 * @brief Stop operator
 */
void STOP(){
    //implement stop
}




/**
 * @brief Compare B to A, sets flags
 */
void _CMP(struct GB_CPU* cpu){

    int i = 0; // temp variable
    i = cpu->_r.a - cpu->_r.b; // subtract b from a without modifying actual a via use of temp variable i also allows for underflow since i is 16 bit instead of 8
    cpu->_r.f = 0x40; //adds to the flag register that the last operation was subtraction
        
    if(!(i & 255)){
        cpu->_r.f |= 0x80; // TLDR: if temp variable = 0 set f to 0
    }

    if(i < 0){
        cpu->_r.f |= 0x10; // Check if subtraction went to negatives, set f to carryover
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

#pragma endregion OPcodes

int main(){
    while(1){
        switch (GCPU._r.pc){
            case 0x00: NOP(&GCPU); break;
            case 0x01: LD_BC(&GCPU); break;
            case 0x02: LD_r16A(&GCPU,GCPU._r.b, GCPU._r.c); break;
            case 0x03: INCr16(&GCPU, GCPU._r.b, GCPU._r.c); break;
            case 0x04: INCr8(&GCPU, GCPU._r.b); break;
            case 0x05: DECr8(&GCPU, GCPU._r.b); break;
            case 0x06: LD_nr8(&GCPU, GCPU._r.b); break;
            case 0x07: RLCA(&GCPU); break;
            case 0x08: LD_n16SP(&GCPU); break;
            case 0x09: ADDHLBC(&GCPU); break;
            case 0x0A: LD_r16A(&GCPU, GCPU._r.b, GCPU._r.c); break;
            case 0x0B: DECr16(&GCPU, GCPU._r.b, GCPU._r.c); break;
            case 0x0C: INCr8(&GCPU, GCPU._r.c); break;
            case 0x0D: DECr8(&GCPU, GCPU._r.c); break;
            case 0x0E: LD_nr8(&GCPU, GCPU._r.c); break;
            case 0x0F: RRCA(&GCPU); break;
            case 0x10: STOP; break;
            case 0x11: LD_DE(&GCPU); break;
            case 0x12: LD_r16A(&GCPU,GCPU._r.d, GCPU._r.e); break;
            case 0x13: INCr16(&GCPU,GCPU._r.d, GCPU._r.e); break;
            case 0x14: INCr8(&GCPU,GCPU._r.d); break;
            case 0x15: DECr8(&GCPU,GCPU._r.d); break;
            case 0x16: LD_nr8(&GCPU, GCPU._r.d); break;
            case 0x17: RLA(&GCPU); break;
            case 0x18:  break;
            case 0x19: ADDHLDE(&GCPU); break;
            case 0x1A: LD_r16A(&GCPU, GCPU._r.d, GCPU._r.e); break;
            case 0x1B: break;
            case 0x1C: break;
            case 0x1D: break;
            case 0x1E: break;
            case 0x1F: break;
            case 0x20: break;
            case 0x21: break;
            case 0x22: break;
            case 0x23: break;
            case 0x24: break;
            case 0x25: break;
            case 0x26: break;
            case 0x27: break;
            case 0x28: break;
            case 0x29: break;
            case 0x2A: break;
            case 0x2B: break;
            case 0x2C: break;
            case 0x2D: break;
            case 0x2E: break;
            case 0x2F: break;
            case 0x30: break;
            case 0x31: break;
            case 0x32: break;
            case 0x33: break;
            case 0x34: break;
            case 0x35: break;
            case 0x36: break;
            case 0x37: break;
            case 0x38: break;
            case 0x39: break;
            case 0x3A: break;
            case 0x3B: break;
            case 0x3C: break;
            case 0x3D: break;
            case 0x3E: break;
            case 0x3F: break;
            case 0x40: break;
            case 0x41: break;
            case 0x42: break;
            case 0x43: break;
            case 0x44: break;
            case 0x45: break;
            case 0x46: break;
            case 0x47: break;
            case 0x48: break;
            case 0x49: break;
            case 0x4A: break;
            case 0x4B: break;
            case 0x4C: break;
            case 0x4D: break;
            case 0x4E: break;
            case 0x4F: break;
            case 0x50: break;
            case 0x51: break;
            case 0x52: break;
            case 0x53: break;
            case 0x54: break;
            case 0x55: break;
            case 0x56: break;
            case 0x57: break;
            case 0x58: break;
            case 0x59: break;
            case 0x5A: break;
            case 0x5B: break;
            case 0x5C: break;
            case 0x5D: break;
            case 0x5E: break;
            case 0x5F: break;
            case 0x60: break;
            case 0x61: break;
            case 0x62: break;
            case 0x63: break;
            case 0x64: break;
            case 0x65: break;
            case 0x66: break;
            case 0x67: break;
            case 0x68: break;
            case 0x69: break;
            case 0x6A: break;
            case 0x6B: break;
            case 0x6C: break;
            case 0x6D: break;
            case 0x6E: break;
            case 0x6F: break;
            case 0x70: break;
            case 0x71: break;
            case 0x72: break;
            case 0x73: break;
            case 0x74: break;
            case 0x75: break;
            case 0x76: break;
            case 0x77: break;
            case 0x78: break;
            case 0x79: break;
            case 0x7A: break;
            case 0x7B: break;
            case 0x7C: break;
            case 0x7D: break;
            case 0x7E: break;
            case 0x7F: break;
            case 0x80: break;
            case 0x81: break;
            case 0x82: break;
            case 0x83: break;
            case 0x84: break;
            case 0x85: break;
            case 0x86: break;
            case 0x87: break;
            case 0x88: break;
            case 0x89: break;
            case 0x8A: break;
            case 0x8B: break;
            case 0x8C: break;
            case 0x8D: break;
            case 0x8E: break;
            case 0x8F: break;
            case 0x90: break;
            case 0x91: break;
            case 0x92: break;
            case 0x93: break;
            case 0x94: break;
            case 0x95: break;
            case 0x96: break;
            case 0x97: break;
            case 0x98: break;
            case 0x99: break;
            case 0x9A: break;
            case 0x9B: break;
            case 0x9C: break;
            case 0x9D: break;
            case 0x9E: break;
            case 0x9F: break;
            case 0xA0: break;
            case 0xA1: break;
            case 0xA2: break;
            case 0xA3: break;
            case 0xA4: break;
            case 0xA5: break;
            case 0xA6: break;
            case 0xA7: break;
            case 0xA8: break;
            case 0xA9: break;
            case 0xAA: break;
            case 0xAB: break;
            case 0xAC: break;
            case 0xAD: break;
            case 0xAE: break;
            case 0xAF: break;
            case 0xB0: break;
            case 0xB1: break;
            case 0xB2: break;
            case 0xB3: break;
            case 0xB4: break;
            case 0xB5: break;
            case 0xB6: break;
            case 0xB7: break;
            case 0xB8: break;
            case 0xB9: break;
            case 0xBA: break;
            case 0xBB: break;
            case 0xBC: break;
            case 0xBD: break;
            case 0xBE: break;
            case 0xBF: break;
            case 0xC0: break;
            case 0xC1: break;
            case 0xC2: break;
            case 0xC3: break;
            case 0xC4: break;
            case 0xC5: break;
            case 0xC6: break;
            case 0xC7: break;
            case 0xC8: break;
            case 0xCA: break;
            case 0xCB: break;
            case 0xCC: break;
            case 0xCD: break;
            case 0xCE: break;
            case 0xCF: break;
            case 0xD0: break;
            case 0xD1: break;
            case 0xD2: break;
            case 0xD3: break;
            case 0xD4: break;
            case 0xD5: break;
            case 0xD6: break;
            case 0xD7: break;
            case 0xD8: break;
            case 0xD9: break;
            case 0xDA: break;
            case 0xDB: break;
            case 0xDC: break;
            case 0xDD: break;
            case 0xDE: break;
            case 0xDF: break;
            case 0xE0: break;
            case 0xE1: break;
            case 0xE2: break;
            case 0xE3: break;
            case 0xE4: break;
            case 0xE5: break;
            case 0xE6: break;
            case 0xE7: break;
            case 0xE8: break;
            case 0xE9: break;
            case 0xEA: break;
            case 0xEB: break;
            case 0xEC: break;
            case 0xED: break;
            case 0xEE: break;
            case 0xEF: break;
            case 0xF0: break;
            case 0xF1: break;
            case 0xF2: break;
            case 0xF3: break;
            case 0xF4: break;
            case 0xF5: break;
            case 0xF6: break;
            case 0xF7: break;
            case 0xF8: break;
            case 0xF9: break;
            case 0xFA: break;
            case 0xFB: break;
            case 0xFC: break;
            case 0xFD: break;
            case 0xFE: break;
            case 0xFF: break;             
        }
    }
}
