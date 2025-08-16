#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>

union Register16
{
    uint16_t x;
    struct
    {
        uint8_t l;
        uint8_t h;
    } bytes;
};

class Registers
{
public:
    Register16 AX, BX, CX, DX;
    uint16_t SI, DI, BP, SP;
    uint16_t CS, DS, ES, SS;
    uint16_t IP;
    uint16_t FLAGS;

    static const uint16_t CF = 0x0001;
    static const uint16_t PF = 0x0004;
    static const uint16_t AF = 0x0010;
    static const uint16_t ZF = 0x0040;
    static const uint16_t SF = 0x0080;
    static const uint16_t TF = 0x0100;
    static const uint16_t IF = 0x0200;
    static const uint16_t DF = 0x0400;
    static const uint16_t OF = 0x0800;

    Registers()
    {
        AX.x = BX.x = CX.x = DX.x = 0;
        SI = DI = BP = 0;
        SP = 0xFFFE;
        CS = DS = ES = SS = 0;
        IP = 0;
        FLAGS = 0x0000;
    }
};

#endif
