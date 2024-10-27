#ifndef EMULATOR8086_H
#define EMULATOR8086_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "registers.h"
#include "memory_components.h"

class Emulator8086
{
private:
    std::vector<uint8_t> memory;
    Registers regs;
    size_t currentInstructionIndex;
    std::map<std::string, std::function<void(std::vector<std::string> &)>> instructions;
    std::map<std::string, uint16_t> labels;
    std::map<std::string, uint16_t> registers;
    std::vector<std::string> program;
    size_t programSize = 0;

    void initializeInstructions();
    uint16_t &getRegister(const std::string &reg);
    uint8_t &getRegister8(const std::string &reg);
    bool is8BitRegister(const std::string &reg);
    bool isMemoryOperand(const std::string &operand);
    uint16_t getValue(const std::string &operand);
    uint8_t getValue8(const std::string &operand);
    void updateFlags(uint16_t result, bool checkCarry = true);
    MemoryOperand parseMemoryOperand(const std::string &operand);
    uint16_t calculateEffectiveAddress(const MemoryOperand &memOp);
    uint16_t readMemoryWord(uint16_t address);
    void writeMemoryWord(uint16_t address, uint16_t value);
    uint8_t readMemoryByte(uint16_t address);
    void writeMemoryByte(uint16_t address, uint8_t value);
    bool isHexNumber(const std::string &str) const;
    bool isDecimalNumber(const std::string &str) const;
    uint16_t parseNumericValue(const std::string &str) const;
    bool is16BitRegister(const std::string &reg);
    bool isRegister(const std::string &reg);
    bool isSegmentRegister(const std::string &reg);
    uint16_t &getSegmentRegister(const std::string &reg);

    void mov(std::vector<std::string> &operands);
    void add(std::vector<std::string> &operands);
    void sub(std::vector<std::string> &operands);
    void mul(std::vector<std::string> &operands);
    void div(std::vector<std::string> &operands);
    void shl(std::vector<std::string> &operands);
    void shr(std::vector<std::string> &operands);
    void push(std::vector<std::string> &operands);
    void pop(std::vector<std::string> &operands);
    void cmp(std::vector<std::string> &operands);
    void test(std::vector<std::string> &operands);
    void jmp(std::vector<std::string> &operands);
    void je(std::vector<std::string> &operands);
    void jne(std::vector<std::string> &operands);
    void jl(std::vector<std::string> &operands);
    void jle(std::vector<std::string> &operands);
    void jg(std::vector<std::string> &operands);
    void jge(std::vector<std::string> &operands);
    void ja(std::vector<std::string> &operands);
    void jae(std::vector<std::string> &operands);
    void jb(std::vector<std::string> &operands);
    void jbe(std::vector<std::string> &operands);
    void inc(std::vector<std::string> &operands);
    void dec(std::vector<std::string> &operands);
    void and_op(std::vector<std::string> &operands);
    void or_op(std::vector<std::string> &operands);
    void xor_op(std::vector<std::string> &operands);
    void not_op(std::vector<std::string> &operands);
    void neg(std::vector<std::string> &operands);
    // void rol(std::vector<std::string> &operands); // New
    // void ror(std::vector<std::string> &operands); // New
    // void call(std::vector<std::string> &operands);   // New
    // void ret(std::vector<std::string> &operands);    // New
    // void int_op(std::vector<std::string> &operands); // New

public:
    explicit Emulator8086(size_t memSize = 1024 * 1024);
    void executeInstruction(const std::string &instruction);
    void displayRegisters();
    void displayStack();
    void displayMemory(uint16_t address, uint16_t count);
    void displayHelp();
};

#endif