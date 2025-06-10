#ifndef PROCESSOR_CONTROL_H
#define PROCESSOR_CONTROL_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086; // Forward declaration

class ProcessorControlInstructions
{
private:
    Emulator8086* emulator;

public:
    ProcessorControlInstructions(Emulator8086* emu);
    
    // Flag control instructions
    void clc(const std::vector<std::string> &operands);
    void cmc(const std::vector<std::string> &operands);
    void stc(const std::vector<std::string> &operands);
    void cld(const std::vector<std::string> &operands);
    void std(const std::vector<std::string> &operands);
    void cli(const std::vector<std::string> &operands);
    void sti(const std::vector<std::string> &operands);
    
    // Processor control instructions
    void hlt(const std::vector<std::string> &operands);
    void wait(const std::vector<std::string> &operands);
    void esc(const std::vector<std::string> &operands);
    void lock(const std::vector<std::string> &operands);
    void nop(const std::vector<std::string> &operands);
    
    // Interrupt instructions
    void int_op(const std::vector<std::string> &operands);
    void into(const std::vector<std::string> &operands);
    void iret(const std::vector<std::string> &operands);
    
    // I/O instructions
    void in_op(const std::vector<std::string> &operands);
    void out(const std::vector<std::string> &operands);
};

#endif
