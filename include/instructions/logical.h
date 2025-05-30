#ifndef LOGICAL_H
#define LOGICAL_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086; // Forward declaration

class LogicalInstructions
{
private:
    Emulator8086* emulator;

public:
    LogicalInstructions(Emulator8086* emu);
    
    // Logical instructions
    void and_op(const std::vector<std::string> &operands);
    void or_op(const std::vector<std::string> &operands);
    void xor_op(const std::vector<std::string> &operands);
    void not_op(const std::vector<std::string> &operands);
    void test(const std::vector<std::string> &operands);
    void cmp(const std::vector<std::string> &operands);
};

#endif
