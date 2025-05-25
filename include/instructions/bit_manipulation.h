#ifndef BIT_MANIPULATION_H
#define BIT_MANIPULATION_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086; // Forward declaration

class BitManipulationInstructions
{
private:
    Emulator8086* emulator;

public:
    BitManipulationInstructions(Emulator8086* emu);
    
    // Rotate and shift instructions
    void rcl(const std::vector<std::string> &operands);
    void rcr(const std::vector<std::string> &operands);
    void rol(const std::vector<std::string> &operands);
    void ror(const std::vector<std::string> &operands);
    void sal(const std::vector<std::string> &operands);
    void sar(const std::vector<std::string> &operands);
    void shl(const std::vector<std::string> &operands);
    void shr(const std::vector<std::string> &operands);
};

#endif
