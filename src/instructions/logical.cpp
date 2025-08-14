#include "instructions/logical.h"
#include "emulator8086.h"
#include <stdexcept>

LogicalInstructions::LogicalInstructions(Emulator8086 *emu) : emulator(emu) {}

void LogicalInstructions::and_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("AND requires 2 operands");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        dest &= src;
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1]))
        {
            uint8_t destVal = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            destVal &= src;
            emulator->writeMemoryByte(address, destVal);
            emulator->updateFlags(destVal, true, false);
        }
        else
        {
            uint16_t destVal = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            destVal &= src;
            emulator->writeMemoryWord(address, destVal);
            emulator->updateFlags(destVal, false, false);
        }
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        dest &= src;
        emulator->updateFlags(dest, false, false);
    }
}

void LogicalInstructions::or_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OR requires 2 operands");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        dest |= src;
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1]))
        {
            uint8_t destVal = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            destVal |= src;
            emulator->writeMemoryByte(address, destVal);
            emulator->updateFlags(destVal, true, false);
        }
        else
        {
            uint16_t destVal = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            destVal |= src;
            emulator->writeMemoryWord(address, destVal);
            emulator->updateFlags(destVal, false, false);
        }
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        dest |= src;
        emulator->updateFlags(dest, false, false);
    }
}

void LogicalInstructions::xor_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XOR requires 2 operands");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        dest ^= src;
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1]))
        {
            uint8_t destVal = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            destVal ^= src;
            emulator->writeMemoryByte(address, destVal);
            emulator->updateFlags(destVal, true, false);
        }
        else
        {
            uint16_t destVal = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            destVal ^= src;
            emulator->writeMemoryWord(address, destVal);
            emulator->updateFlags(destVal, false, false);
        }
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        dest ^= src;
        emulator->updateFlags(dest, false, false);
    }
}

void LogicalInstructions::not_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("NOT requires 1 operand");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        dest = ~dest;
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t destVal = emulator->readMemoryWord(address);
        destVal = ~destVal;
        emulator->writeMemoryWord(address, destVal);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        dest = ~dest;
    }
}

void LogicalInstructions::test(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("TEST requires 2 operands");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint8_t result = dest & src;
        emulator->updateFlags(result, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1]))
        {
            uint8_t dest = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            uint8_t result = dest & src;
            emulator->updateFlags(result, true, false);
        }
        else
        {
            uint16_t dest = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            uint16_t result = dest & src;
            emulator->updateFlags(result, false, false);
        }
    }
    else
    {
        uint16_t dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint16_t result = dest & src;
        emulator->updateFlags(result, false, false);
    }
}

void LogicalInstructions::cmp(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("CMP requires 2 operands");
    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint16_t result = dest - src;
        emulator->updateFlags(result, true, true);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1]))
        {
            uint8_t dest = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            uint16_t result = dest - src;
            emulator->updateFlags(result, true, true);
        }
        else
        {
            uint16_t dest = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            uint32_t result = dest - src;
            emulator->updateFlags(result, false, true);
        }
    }
    else
    {
        uint16_t dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint32_t result = dest - src;
        emulator->updateFlags(result, false, true);
    }
}
