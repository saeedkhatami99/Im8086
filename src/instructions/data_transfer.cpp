#include "instructions/data_transfer.h"
#include "emulator8086.h"
#include <stdexcept>
#include <algorithm>

DataTransferInstructions::DataTransferInstructions(Emulator8086* emu) : emulator(emu) {}

void DataTransferInstructions::mov(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("MOV requires 2 operands");
    const std::string &dest = operands[0];
    const std::string &src = operands[1];

    if (emulator->is8BitRegister(dest))
    {
        emulator->getRegister8(dest) = emulator->getValue8(src);
    }
    else if (emulator->isMemoryOperand(dest))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(dest);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(src))
            emulator->writeMemoryByte(address, emulator->getRegister8(src));
        else
            emulator->writeMemoryWord(address, emulator->getValue(src));
    }
    else
    {
        emulator->getRegister(dest) = emulator->getValue(src);
    }
}

void DataTransferInstructions::push(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("PUSH requires 1 operand");
    if (emulator->is8BitRegister(operands[0]))
        throw std::runtime_error("PUSH requires 16-bit operand");
    uint16_t value = emulator->getValue(operands[0]);
    emulator->getRegisters().SP -= 2;
    emulator->writeMemoryWord(emulator->getRegisters().SP, value);
}

void DataTransferInstructions::pop(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("POP requires 1 operand");
    if (emulator->is8BitRegister(operands[0]))
        throw std::runtime_error("POP requires 16-bit operand");
    uint16_t value = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        emulator->writeMemoryWord(emulator->calculateEffectiveAddress(memOp), value);
    }
    else
    {
        emulator->getRegister(operands[0]) = value;
    }
}

void DataTransferInstructions::xchg(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XCHG requires 2 operands");
    
    if (emulator->is8BitRegister(operands[0]) && emulator->is8BitRegister(operands[1]))
    {
        uint8_t temp = emulator->getRegister8(operands[0]);
        emulator->getRegister8(operands[0]) = emulator->getRegister8(operands[1]);
        emulator->getRegister8(operands[1]) = temp;
    }
    else if (!emulator->isMemoryOperand(operands[0]) && !emulator->isMemoryOperand(operands[1]))
    {
        uint16_t temp = emulator->getRegister(operands[0]);
        emulator->getRegister(operands[0]) = emulator->getRegister(operands[1]);
        emulator->getRegister(operands[1]) = temp;
    }
    else
    {
        
        if (emulator->isMemoryOperand(operands[0]) && !emulator->isMemoryOperand(operands[1]))
        {
            
            MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
            uint16_t address = emulator->calculateEffectiveAddress(memOp);
            if (emulator->is8BitRegister(operands[1]))
            {
                uint8_t temp = emulator->readMemoryByte(address);
                emulator->writeMemoryByte(address, emulator->getRegister8(operands[1]));
                emulator->getRegister8(operands[1]) = temp;
            }
            else
            {
                uint16_t temp = emulator->readMemoryWord(address);
                emulator->writeMemoryWord(address, emulator->getRegister(operands[1]));
                emulator->getRegister(operands[1]) = temp;
            }
        }
        else if (!emulator->isMemoryOperand(operands[0]) && emulator->isMemoryOperand(operands[1]))
        {
            
            MemoryOperand memOp = emulator->parseMemoryOperand(operands[1]);
            uint16_t address = emulator->calculateEffectiveAddress(memOp);
            if (emulator->is8BitRegister(operands[0]))
            {
                uint8_t temp = emulator->getRegister8(operands[0]);
                emulator->getRegister8(operands[0]) = emulator->readMemoryByte(address);
                emulator->writeMemoryByte(address, temp);
            }
            else
            {
                uint16_t temp = emulator->getRegister(operands[0]);
                emulator->getRegister(operands[0]) = emulator->readMemoryWord(address);
                emulator->writeMemoryWord(address, temp);
            }
        }
        else
        {
            throw std::runtime_error("XCHG between two memory operands not supported");
        }
    }
}

void DataTransferInstructions::lea(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LEA requires 2 operands");
    if (!emulator->isMemoryOperand(operands[1]))
        throw std::runtime_error("LEA requires memory source");
    MemoryOperand memOp = emulator->parseMemoryOperand(operands[1]);
    emulator->getRegister(operands[0]) = emulator->calculateEffectiveAddress(memOp);
}

void DataTransferInstructions::lds(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LDS requires 2 operands");
    if (!emulator->isMemoryOperand(operands[1]))
        throw std::runtime_error("LDS requires memory source");
    MemoryOperand memOp = emulator->parseMemoryOperand(operands[1]);
    uint16_t address = emulator->calculateEffectiveAddress(memOp);
    emulator->getRegister(operands[0]) = emulator->readMemoryWord(address);
    emulator->getRegisters().DS = emulator->readMemoryWord(address + 2);
}

void DataTransferInstructions::les(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LES requires 2 operands");
    if (!emulator->isMemoryOperand(operands[1]))
        throw std::runtime_error("LES requires memory source");
    MemoryOperand memOp = emulator->parseMemoryOperand(operands[1]);
    uint16_t address = emulator->calculateEffectiveAddress(memOp);
    emulator->getRegister(operands[0]) = emulator->readMemoryWord(address);
    emulator->getRegisters().ES = emulator->readMemoryWord(address + 2);
}

void DataTransferInstructions::lahf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LAHF takes no operands");
    emulator->getRegisters().AX.h = emulator->getRegisters().FLAGS & 0xFF;
}

void DataTransferInstructions::sahf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SAHF takes no operands");
    emulator->getRegisters().FLAGS = (emulator->getRegisters().FLAGS & 0xFF00) | (emulator->getRegisters().AX.h & 0xFF);
}

void DataTransferInstructions::pushf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("PUSHF takes no operands");
    emulator->getRegisters().SP -= 2;
    emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().FLAGS);
}

void DataTransferInstructions::popf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("POPF takes no operands");
    emulator->getRegisters().FLAGS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
}

void DataTransferInstructions::pusha(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("PUSHA takes no operands");
    uint16_t tempSP = emulator->getRegisters().SP;
    push({"AX"});
    push({"CX"});
    push({"DX"});
    push({"BX"});
    emulator->getRegisters().SP -= 2;
    emulator->writeMemoryWord(emulator->getRegisters().SP, tempSP);
    push({"BP"});
    push({"SI"});
    push({"DI"});
}

void DataTransferInstructions::popa(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("POPA takes no operands");
    pop({"DI"});
    pop({"SI"});
    pop({"BP"});
    emulator->getRegisters().SP += 2; 
    pop({"BX"});
    pop({"DX"});
    pop({"CX"});
    pop({"AX"});
}
