#include "registers.h"

Registers::Registers() {
    AX = BX = CX = DX = 0;
    SI = DI = 0;
    BP = 0;
    SP = 0xFFFE;  // Initialize stack pointer to top of memory
    CS = DS = ES = SS = 0;
    IP = 0;
    FLAGS = 0x0000;
}