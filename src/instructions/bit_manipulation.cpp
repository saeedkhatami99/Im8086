#include "instructions/bit_manipulation.h"
#include "emulator8086.h"
#include <stdexcept>

BitManipulationInstructions::BitManipulationInstructions(Emulator8086 *emu) : emulator(emu) {}

void BitManipulationInstructions::rcl(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("RCL requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;
    bool carry = emulator->getRegisters().FLAGS & Registers::CF;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x80;
            dest = (dest << 1) | (carry ? 1 : 0);
            carry = newCarry;
        }
        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x8000;
            dest = (dest << 1) | (carry ? 1 : 0);
            carry = newCarry;
        }
        emulator->writeMemoryWord(address, dest);

        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x8000;
            dest = (dest << 1) | (carry ? 1 : 0);
            carry = newCarry;
        }
        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
}

void BitManipulationInstructions::rcr(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("RCR requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;
    bool carry = emulator->getRegisters().FLAGS & Registers::CF;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x01;
            dest = (dest >> 1) | (carry ? 0x80 : 0);
            carry = newCarry;
        }
        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x0001;
            dest = (dest >> 1) | (carry ? 0x8000 : 0);
            carry = newCarry;
        }
        emulator->writeMemoryWord(address, dest);

        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool newCarry = dest & 0x0001;
            dest = (dest >> 1) | (carry ? 0x8000 : 0);
            carry = newCarry;
        }
        if (carry)
            emulator->getRegisters().FLAGS |= Registers::CF;
        else
            emulator->getRegisters().FLAGS &= ~Registers::CF;
    }
}

void BitManipulationInstructions::rol(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ROL requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x80;
            dest = (dest << 1) | (carry ? 1 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x8000;
            dest = (dest << 1) | (carry ? 1 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->writeMemoryWord(address, dest);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x8000;
            dest = (dest << 1) | (carry ? 1 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
    }
}

void BitManipulationInstructions::ror(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ROR requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x01;
            dest = (dest >> 1) | (carry ? 0x80 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest = (dest >> 1) | (carry ? 0x8000 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->writeMemoryWord(address, dest);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest = (dest >> 1) | (carry ? 0x8000 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
    }
}

void BitManipulationInstructions::sal(const std::vector<std::string> &operands)
{
    shl(operands);
}

void BitManipulationInstructions::sar(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SAR requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        bool sign = dest & 0x80;
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x01;
            dest = (dest >> 1) | (sign ? 0x80 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);
        bool sign = dest & 0x8000;

        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest = (dest >> 1) | (sign ? 0x8000 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->writeMemoryWord(address, dest);
        emulator->updateFlags(dest, false, false);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        bool sign = dest & 0x8000;
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest = (dest >> 1) | (sign ? 0x8000 : 0);
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, false, false);
    }
}

void BitManipulationInstructions::shl(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHL requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x80;
            dest <<= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x8000;
            dest <<= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->writeMemoryWord(address, dest);
        emulator->updateFlags(dest, false, false);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x8000;
            dest <<= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, false, false);
    }
}

void BitManipulationInstructions::shr(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHR requires 2 operands");

    uint8_t count = emulator->getValue8(operands[1]) & 0x1F;

    if (emulator->is8BitRegister(operands[0]))
    {
        uint8_t &dest = emulator->getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x01;
            dest >>= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, true, false);
    }
    else if (emulator->isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t dest = emulator->readMemoryWord(address);

        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest >>= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->writeMemoryWord(address, dest);
        emulator->updateFlags(dest, false, false);
    }
    else
    {
        uint16_t &dest = emulator->getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool carry = dest & 0x0001;
            dest >>= 1;
            if (carry)
                emulator->getRegisters().FLAGS |= Registers::CF;
            else
                emulator->getRegisters().FLAGS &= ~Registers::CF;
        }
        emulator->updateFlags(dest, false, false);
    }
}
