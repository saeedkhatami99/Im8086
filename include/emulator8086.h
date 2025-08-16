#ifndef EMULATOR8086_H
#define EMULATOR8086_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include "registers.h"
#include "memory_components.h"

class DataTransferInstructions;
class ArithmeticInstructions;
class LogicalInstructions;
class StringInstructions;
class ProgramTransferInstructions;
class ProcessorControlInstructions;
class BitManipulationInstructions;

class Emulator8086
{
private:
    Registers regs;
    std::vector<uint8_t> memory;

    std::map<std::string, std::function<void(std::vector<std::string> &)>> instructions;
    std::map<std::string, size_t> labels;
    std::vector<std::string> program;

    std::unique_ptr<DataTransferInstructions> dataTransfer;
    std::unique_ptr<ArithmeticInstructions> arithmetic;
    std::unique_ptr<LogicalInstructions> logical;
    std::unique_ptr<StringInstructions> string;
    std::unique_ptr<ProgramTransferInstructions> programTransfer;
    std::unique_ptr<ProcessorControlInstructions> processorControl;
    std::unique_ptr<BitManipulationInstructions> bitManipulation;

    void initializeInstructions();

public:
    explicit Emulator8086(size_t memSize = 1024 * 1024);
    ~Emulator8086();

    void executeInstruction(const std::string &instruction);

    void loadProgram(const std::vector<std::string> &lines);
    bool step();
    void reset();
    const std::vector<std::string> &getProgram() const { return program; }
    size_t getIP() const { return regs.IP; }
    void setIP(size_t ip) { regs.IP = static_cast<uint16_t>(ip); }
    void displayRegisters();
    void displayStack();
    void displayMemory(uint16_t address, uint16_t count);
    void displayHelp();

    uint16_t &getRegister(const std::string &reg);
    uint8_t &getRegister8(const std::string &reg);
    bool is8BitRegister(const std::string &reg);
    bool isMemoryOperand(const std::string &operand);
    uint16_t getValue(const std::string &operand);
    uint8_t getValue8(const std::string &operand);
    void updateFlags(uint32_t result, bool isByte, bool checkCarry);
    MemoryOperand parseMemoryOperand(const std::string &operand);
    uint16_t calculateEffectiveAddress(const MemoryOperand &memOp);
    uint16_t readMemoryWord(uint16_t address);
    void writeMemoryWord(uint16_t address, uint16_t value);
    uint8_t readMemoryByte(uint16_t address);
    void writeMemoryByte(uint16_t address, uint8_t value);

    Registers &getRegisters() { return regs; }
    std::vector<uint8_t> &getMemory() { return memory; }
    std::map<std::string, size_t> &getLabels() { return labels; }
    
    size_t getLabelAddress(const std::string &label);
    bool hasLabel(const std::string &label);
};

#endif
