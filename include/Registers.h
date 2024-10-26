#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>

struct Registers {
    uint16_t AX, BX, CX, DX;    // General purpose registers
    uint16_t SI, DI;            // Index registers
    uint16_t BP, SP;            // Base and Stack pointers
    uint16_t CS, DS, ES, SS;    // Segment registers
    uint16_t IP;                // Instruction pointer
    uint16_t FLAGS;             // Flags register
    
    // Flag bit positions
    static const uint16_t CF = 0x0001; // Carry Flag
    static const uint16_t PF = 0x0004; // Parity Flag
    static const uint16_t AF = 0x0010; // Auxiliary Flag
    static const uint16_t ZF = 0x0040; // Zero Flag
    static const uint16_t SF = 0x0080; // Sign Flag
    static const uint16_t OF = 0x0800; // Overflow Flag

    Registers();
};

#endif