#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>

struct Registers {
    uint16_t AX, BX, CX, DX;
    uint16_t SI, DI;
    uint16_t BP, SP;
    uint16_t CS, DS, ES, SS;
    uint16_t IP;
    uint16_t FLAGS;
    
    static const uint16_t CF = 0x0001;
    static const uint16_t PF = 0x0004;
    static const uint16_t AF = 0x0010;
    static const uint16_t ZF = 0x0040;
    static const uint16_t SF = 0x0080;
    static const uint16_t OF = 0x0800;

    Registers();
};

#endif