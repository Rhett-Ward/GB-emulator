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

*/


/* Summary of file:
Gameboy CPU, this is where all the opcodes are, where the flags are defined, registers, cpu clock, etc
*/

//Includes needed
#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16 
#include "MMU.h" // memory manipulation unit interface
#include "GB CPU.h" // blueprint for this file 



//opcodes / instruction functions:
#pragma region OPcodes

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

#pragma endregion SUB functions

#pragma region AND functions

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


#pragma endregion AND funtions

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
 *@brief Copy the value of n into r8 AKA LD(r8,n)
 *@param cpu pointer to GB CPU
 *@param r8 pointer to register
 *@param r82 pointer to register
 bc de hl
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
        cpu->_r.pc+=2;
    }
}

/**
 * @brief Jump to address n16 if the Z flag is not checked
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