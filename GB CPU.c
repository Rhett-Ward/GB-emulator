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
*/


/* Summary of file:
Gameboy CPU, this is where all the opcodes are, where the flags are defined, registers, cpu clock, etc
*/

//Includes needed
#include <stdio.h> // standard in out
#include <stdint.h> // uint8 and uint16 
//#include "MMU.c" // memory manipulation unit interface


//FLAGS
#define Z_FLAG 0x80 // bit 7 (zero flag)
#define N_FLAG 0x40 // bit 6 (subtraction flag)
#define H_FLAG 0x20 // bit 5 (Half Carry Flag)
#define C_FLAG 0x10 // bit 4 (Carry Flag)
/*
 * will take a minute here to explain the gameboy flag system as i understand it
 
 f is a uint8_t register that holds flags based on the first 4 bits of the 8 bits

 hexadecimal notation "0x##" is a 16 bit notation that is translated to 8 bits by breaking the two numbers after "0x" into their 4 bit equivalents

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
};

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
}


/** WIP
 * @brief Add HL to A, Leaves result in A AKA (ADD A, HL)
 * @param cpu point to GB_CPU
 */
void ADD_hl2a(struct GB_CPU* cpu){
    
    uint16_t i = 0; //temp variable that can hold overflow

    // Need MMU to write // Add HL to A, order doesnt matter bc addition
    /*
    cpu->_r.f = 0; // Clear flags

    if(!(i & 255)){
        cpu->_r.f |= Z_FLAG; // TLDR: if a = 0 set f to 0, 0x80 is the zero denotation, the if checks if register a is a value b/t 1-255 if not then proceed.
    }

    if(i > 255){
        cpu->_r.f |= C_FLAG; //TLDR: f = a - 255, 0x10 is the carry operator which means its equal to the leftovers of the last operation that went over 255 or under 0, check is simple.
    }

    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    */
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
}

/** WIP
 * @brief Add n to SP, Leaves result in SP AKA (ADD SP, d)
 * @param cpu point to GB_CPU
 * needs memory not done
 */
void ADD_n2a(struct GB_CPU* cpu){
    
    uint16_t i = 0; //temp variable that can hold overflow

    // Need MMU to write

    /*
    cpu->_r.a = (uint8_t)i; //sets result to a 
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
    */
}

#pragma endregion ADD funtions

#pragma region SUB functions

/**
 * @brief Subtracts B from A storing the result in A, AKA (SUB B,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2b(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.b;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & C_FLAG){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts C from A storing the result in A, AKA (SUB B,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2c(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.c;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts D from A storing the result in A, AKA (SUB B,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2d(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.d;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts E from A storing the result in A, AKA (SUB B,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2e(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.e;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts H from A storing the result in A, AKA (SUB H,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2h(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.h;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts L from A storing the result in A, AKA (SUB L,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2l(struct GB_CPU* cpu){
    uint8_t a = cpu->_r.a;
    uint8_t l = cpu->_r.l;

    uint16_t temp = a - l;
    uint8_t result = (uint8_t)temp;

    cpu->_r.f |= 0x80; // subtraction flag

    if(temp < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }
        
    if((cpu->_r.a ^ cpu->_r.l ^ a) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Subtracts A from A storing the result in A, AKA (SUB A,A)
 * @param cpu pointer for GB CPU
 */
void SUB_a2a(struct GB_CPU* cpu){
    uint16_t i = 0; //temp variable

    i = cpu->_r.a - cpu->_r.a;

    if(i < 0){ // underflow check
        cpu->_r.f = 0x50; // half carry plus some other flag
    }
    else{
        cpu->_r.f = 0x40; // subtraction flag
    }

    if(!(i &= 255)){
        cpu->_r.f |= 0x80; // zero flag
    }
    if((cpu->_r.a ^ cpu->_r.b ^ i) & 0x10){
        cpu->_r.f |= 0x20; // halfcarry flag
    }

    cpu->_r.a = (uint8_t)i;
    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}


#pragma endregion SUB functions

#pragma region AND functions

/**
 * @brief bitwise and of A and B and stores in A
 * @param cpu pointer to GB CPU
 */
void AND_a2b(struct GB_CPU* cpu){
    cpu->_r.a&=cpu->_r.b; // bitwise comparison

    if (cpu->_r.a == 0) {
        cpu->_r.f = 0x80; // if = 0 then set 0 flag
    } else {
        cpu->_r.f = 0; // clear flags
    }

    cpu->_r.m = 1; cpu->_r.t = 4; //time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}


#pragma endregion AND funtions




















/**
 * @brief Function for do nothing.
 */
void NOP(struct GB_CPU* cpu){
    cpu->_r.m = 1; cpu->_r.t = 4; //Time of last cycle
    cpu->_c.m += cpu->_r.m; cpu->_c.t += cpu->_r.t; //Total time of cycles
}

/**
 * @brief Stop operator
 */
void STOP(){
    //implement stop
}

/**
 * @brief relative jump by signed immediate if last result was not zero
 */
void JRNZn(){
    // return after making MMU
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