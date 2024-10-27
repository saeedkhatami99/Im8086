#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <bitset>
#include <functional>
#include "emulator8086.h"

Emulator8086::Emulator8086(size_t memSize) : memory(memSize, 0)
{
    initializeInstructions();
}

void Emulator8086::initializeInstructions()
{
    instructions["MOV"] = std::bind(&Emulator8086::mov, this, std::placeholders::_1);
    instructions["ADD"] = std::bind(&Emulator8086::add, this, std::placeholders::_1);
    instructions["SUB"] = std::bind(&Emulator8086::sub, this, std::placeholders::_1);
    instructions["MUL"] = std::bind(&Emulator8086::mul, this, std::placeholders::_1);
    instructions["DIV"] = std::bind(&Emulator8086::div, this, std::placeholders::_1);
    instructions["SHL"] = std::bind(&Emulator8086::shl, this, std::placeholders::_1);
    instructions["SHR"] = std::bind(&Emulator8086::shr, this, std::placeholders::_1);
    instructions["PUSH"] = std::bind(&Emulator8086::push, this, std::placeholders::_1);
    instructions["POP"] = std::bind(&Emulator8086::pop, this, std::placeholders::_1);
    instructions["CMP"] = std::bind(&Emulator8086::cmp, this, std::placeholders::_1);
    instructions["TEST"] = std::bind(&Emulator8086::test, this, std::placeholders::_1);
    instructions["JMP"] = std::bind(&Emulator8086::jmp, this, std::placeholders::_1);
    instructions["JE"] = std::bind(&Emulator8086::je, this, std::placeholders::_1);
    instructions["JNE"] = std::bind(&Emulator8086::jne, this, std::placeholders::_1);
    instructions["JL"] = std::bind(&Emulator8086::jl, this, std::placeholders::_1);
    instructions["JLE"] = std::bind(&Emulator8086::jle, this, std::placeholders::_1);
    instructions["JG"] = std::bind(&Emulator8086::jg, this, std::placeholders::_1);
    instructions["JGE"] = std::bind(&Emulator8086::jge, this, std::placeholders::_1);
    instructions["JA"] = std::bind(&Emulator8086::ja, this, std::placeholders::_1);
    instructions["JAE"] = std::bind(&Emulator8086::jae, this, std::placeholders::_1);
    instructions["JB"] = std::bind(&Emulator8086::jb, this, std::placeholders::_1);
    instructions["JBE"] = std::bind(&Emulator8086::jbe, this, std::placeholders::_1);
    instructions["INC"] = std::bind(&Emulator8086::inc, this, std::placeholders::_1);
    instructions["DEC"] = std::bind(&Emulator8086::dec, this, std::placeholders::_1);
    instructions["AND"] = std::bind(&Emulator8086::and_op, this, std::placeholders::_1);
    instructions["OR"] = std::bind(&Emulator8086::or_op, this, std::placeholders::_1);
    instructions["XOR"] = std::bind(&Emulator8086::xor_op, this, std::placeholders::_1);
    instructions["NOT"] = std::bind(&Emulator8086::not_op, this, std::placeholders::_1);
    instructions["NEG"] = std::bind(&Emulator8086::neg, this, std::placeholders::_1);
    // instructions["ROR"] = std::bind(&Emulator8086::ror_op, this, std::placeholders::_1);
    // instructions["ROL"] = std::bind(&Emulator8086::rol_op, this, std::placeholders::_1);
    // instructions["CALL"] = std::bind(&Emulator8086::call, this, std::placeholders::_1);
    // instructions["RET"] = std::bind(&Emulator8086::ret, this, std::placeholders::_1);
    // instructions["INT"] = std::bind(&Emulator8086::int_op, this, std::placeholders::_1);
}

uint16_t &Emulator8086::getRegister(const std::string &reg)
{
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);

    if (upperReg == "AX")
        return regs.AX.x;
    if (upperReg == "BX")
        return regs.BX.x;
    if (upperReg == "CX")
        return regs.CX.x;
    if (upperReg == "DX")
        return regs.DX.x;
    if (upperReg == "SI")
        return regs.SI;
    if (upperReg == "DI")
        return regs.DI;
    if (upperReg == "BP")
        return regs.BP;
    if (upperReg == "SP")
        return regs.SP;
    throw std::runtime_error("Invalid 16-bit register: " + reg);
}

uint8_t &Emulator8086::getRegister8(const std::string &reg)
{
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);

    if (upperReg == "AL")
        return regs.AX.l;
    if (upperReg == "AH")
        return regs.AX.h;
    if (upperReg == "BL")
        return regs.BX.l;
    if (upperReg == "BH")
        return regs.BX.h;
    if (upperReg == "CL")
        return regs.CX.l;
    if (upperReg == "CH")
        return regs.CX.h;
    if (upperReg == "DL")
        return regs.DX.l;
    if (upperReg == "DH")
        return regs.DX.h;
    throw std::runtime_error("Invalid 8-bit register: " + reg);
}

bool Emulator8086::is8BitRegister(const std::string &reg)
{
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);

    return (upperReg == "AL" || upperReg == "AH" || upperReg == "BL" || upperReg == "BH" ||
            upperReg == "CL" || upperReg == "CH" || upperReg == "DL" || upperReg == "DH");
}

bool Emulator8086::is16BitRegister(const std::string &reg)
{
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);

    return (upperReg == "AX" || upperReg == "BX" || upperReg == "CX" || upperReg == "DX" ||
            upperReg == "SI" || upperReg == "DI" || upperReg == "BP" || upperReg == "SP");
}

bool Emulator8086::isRegister(const std::string &reg)
{
    return is16BitRegister(reg) || is8BitRegister(reg);
}

bool Emulator8086::isHexNumber(const std::string &str) const
{
    if (str.empty())
        return false;

    std::string value = str;
    if (str.back() == 'h' || str.back() == 'H')
    {
        value = str.substr(0, str.length() - 1);
    }
    else
    {
        return false;
    }

    return std::all_of(value.begin(), value.end(), [](char c)
                       { return std::isxdigit(c); });
}

bool Emulator8086::isDecimalNumber(const std::string &str) const
{
    return std::all_of(str.begin(), str.end(), [](char c)
                       { return std::isdigit(c); });
}

uint16_t Emulator8086::parseNumericValue(const std::string &str) const
{
    if (str.empty())
    {
        throw std::runtime_error("Empty string provided for numeric value");
    }

    if (isHexNumber(str))
    {
        std::string hexValue = str.substr(0, str.length() - 1);
        return static_cast<uint16_t>(std::stoul(hexValue, nullptr, 16));
    }

    if (isDecimalNumber(str))
    {
        return static_cast<uint16_t>(std::stoul(str, nullptr, 10));
    }

    throw std::runtime_error("Invalid number format: " + str);
}

MemoryOperand Emulator8086::parseMemoryOperand(const std::string &operand)
{
    MemoryOperand result;
    std::string inner = operand.substr(1, operand.length() - 2);

    std::vector<std::string> parts;
    std::string current;
    bool negative = false;

    for (size_t i = 0; i < inner.length(); i++)
    {
        if (inner[i] == '+' || inner[i] == '-')
        {
            if (!current.empty())
            {
                parts.push_back(current);
                current.clear();
            }
            if (inner[i] == '-')
                negative = true;
        }
        else
        {
            current += inner[i];
        }
    }
    if (!current.empty())
    {
        parts.push_back(current);
    }

    for (const auto &part : parts)
    {
        std::string upperPart = part;
        std::transform(upperPart.begin(), upperPart.end(), upperPart.begin(), ::toupper);

        if (upperPart == "BX")
        {
            result.hasBase = true;
            result.base = regs.BX.x;
        }
        else if (upperPart == "BP")
        {
            result.hasBase = true;
            result.base = regs.BP;
        }
        else if (upperPart == "SI")
        {
            result.hasIndex = true;
            result.index = regs.SI;
        }
        else if (upperPart == "DI")
        {
            result.hasIndex = true;
            result.index = regs.DI;
        }
        else
        {
            result.hasDisplacement = true;
            result.displacement = std::stoi(part, nullptr, 16);
            if (negative)
                result.displacement = -result.displacement;
        }
    }

    return result;
}

uint16_t Emulator8086::calculateEffectiveAddress(const MemoryOperand &memOp)
{
    uint16_t ea = 0;

    if (memOp.hasBase)
    {
        ea += memOp.base;
    }

    if (memOp.hasIndex)
    {
        ea += memOp.index;
    }

    if (memOp.hasDisplacement)
    {
        ea += memOp.displacement;
    }

    return ea;
}

uint16_t Emulator8086::readMemoryWord(uint16_t address)
{
    return (memory[address + 1] << 8) | memory[address];
}

void Emulator8086::writeMemoryWord(uint16_t address, uint16_t value)
{
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

uint8_t Emulator8086::readMemoryByte(uint16_t address)
{
    return memory[address];
}

void Emulator8086::writeMemoryByte(uint16_t address, uint8_t value)
{
    memory[address] = value;
}

bool Emulator8086::isMemoryOperand(const std::string &operand)
{
    return operand[0] == '[' && operand.back() == ']';
}

uint16_t Emulator8086::getValue(const std::string &operand)
{
    std::string upperOperand = operand;
    std::transform(upperOperand.begin(), upperOperand.end(), upperOperand.begin(), ::toupper);

    if (registers.find(upperOperand) != registers.end())
    {
        return getRegister(upperOperand);
    }

    if (isMemoryOperand(operand))
    {
        MemoryOperand memOp = parseMemoryOperand(operand);
        uint16_t address = calculateEffectiveAddress(memOp);
        return readMemoryWord(address);
    }

    return parseNumericValue(operand);
}

void Emulator8086::updateFlags(uint16_t result, bool checkCarry)
{
    if (result == 0)
    {
        regs.FLAGS |= Registers::ZF;
    }
    else
    {
        regs.FLAGS &= ~Registers::ZF;
    }

    if (result & 0x8000)
    {
        regs.FLAGS |= Registers::SF;
    }
    else
    {
        regs.FLAGS &= ~Registers::SF;
    }

    if (checkCarry && (result > 0xFFFF))
    {
        regs.FLAGS |= Registers::CF;
    }
    else if (checkCarry)
    {
        regs.FLAGS &= ~Registers::CF;
    }
}

void Emulator8086::mov(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("MOV requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (isSegmentRegister(dest))
    {
        if (isSegmentRegister(source))
        {
            setSegmentRegister(dest, getSegmentRegister(source));
        }
        else if (isRegister(source))
        {
            setSegmentRegister(dest, getRegister(source));
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            setSegmentRegister(dest, readMemoryWord(address));
        }
        else
        {
            setSegmentRegister(dest, parseNumericValue(source));
        }
        return;
    }

    if (isSegmentRegister(source))
    {
        if (isRegister(dest))
        {
            getRegister(dest) = getSegmentRegister(source);
        }
        else if (isMemoryOperand(dest))
        {
            MemoryOperand memOp = parseMemoryOperand(dest);
            uint16_t address = calculateEffectiveAddress(memOp);
            writeMemoryWord(address, getSegmentRegister(source));
        }
        return;
    }

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        if (is8BitRegister(source))
        {
            destReg = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            destReg = readMemoryByte(address);
        }
        else
        {
            destReg = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }
        return;
    }

    if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            writeMemoryWord(address, readMemoryWord(srcAddress));
        }
        else
        {
            writeMemoryWord(address, getValue(source));
        }
    }
    else
    {
        uint16_t &destReg = getRegister(dest);
        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            destReg = readMemoryWord(srcAddress);
        }
        else
        {
            destReg = getValue(source);
        }
    }
}

void Emulator8086::add(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ADD requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            sourceValue = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }

        uint16_t result = static_cast<uint16_t>(destReg) + static_cast<uint16_t>(sourceValue);

        bool carryOut = (result > 0xFF);
        bool overflow = ((destReg ^ result) & (sourceValue ^ result) & 0x80) != 0;
        bool auxCarry = ((destReg & 0x0F) + (sourceValue & 0x0F)) > 0x0F;

        destReg = static_cast<uint8_t>(result & 0xFF);

        if (carryOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxCarry)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(destReg, false);
    }

    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        uint16_t destValue = readMemoryWord(address);
        uint16_t sourceValue;

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint32_t result = static_cast<uint32_t>(destValue) + static_cast<uint32_t>(sourceValue);

        bool carryOut = (result > 0xFFFF);
        bool overflow = ((destValue ^ result) & (sourceValue ^ result) & 0x8000) != 0;
        bool auxCarry = ((destValue & 0x0F) + (sourceValue & 0x0F)) > 0x0F;

        writeMemoryWord(address, static_cast<uint16_t>(result & 0xFFFF));

        if (carryOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxCarry)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(static_cast<uint16_t>(result), false);
    }

    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t sourceValue;

        if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint32_t result = static_cast<uint32_t>(destReg) + static_cast<uint32_t>(sourceValue);

        bool carryOut = (result > 0xFFFF);
        bool overflow = ((destReg ^ result) & (sourceValue ^ result) & 0x8000) != 0;
        bool auxCarry = ((destReg & 0x0F) + (sourceValue & 0x0F)) > 0x0F;

        destReg = static_cast<uint16_t>(result & 0xFFFF);

        if (carryOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxCarry)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(destReg, false);
    }
}

void Emulator8086::sub(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SUB requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            sourceValue = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }

        uint8_t oldDest = destReg;
        uint16_t result = static_cast<uint16_t>(destReg) - static_cast<uint16_t>(sourceValue);

        bool borrowOut = (destReg < sourceValue);
        bool overflow = ((oldDest ^ sourceValue) & (oldDest ^ result) & 0x80) != 0;
        bool auxBorrow = (oldDest & 0x0F) < (sourceValue & 0x0F);

        destReg = static_cast<uint8_t>(result & 0xFF);

        if (borrowOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxBorrow)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(destReg, false);
    }

    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        uint16_t destValue = readMemoryWord(address);
        uint16_t sourceValue;

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t oldDest = destValue;
        uint32_t result = static_cast<uint32_t>(destValue) - static_cast<uint32_t>(sourceValue);

        bool borrowOut = (destValue < sourceValue);
        bool overflow = ((oldDest ^ sourceValue) & (oldDest ^ result) & 0x8000) != 0;
        bool auxBorrow = (oldDest & 0x0F) < (sourceValue & 0x0F);

        writeMemoryWord(address, static_cast<uint16_t>(result & 0xFFFF));

        if (borrowOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxBorrow)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(static_cast<uint16_t>(result), false);
    }

    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t sourceValue;

        if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t oldDest = destReg;
        uint32_t result = static_cast<uint32_t>(destReg) - static_cast<uint32_t>(sourceValue);

        bool borrowOut = (destReg < sourceValue);
        bool overflow = ((oldDest ^ sourceValue) & (oldDest ^ result) & 0x8000) != 0;
        bool auxBorrow = (oldDest & 0x0F) < (sourceValue & 0x0F);

        destReg = static_cast<uint16_t>(result & 0xFFFF);

        if (borrowOut)
            regs.FLAGS |= Registers::CF;
        else
            regs.FLAGS &= ~Registers::CF;

        if (overflow)
            regs.FLAGS |= Registers::OF;
        else
            regs.FLAGS &= ~Registers::OF;

        if (auxBorrow)
            regs.FLAGS |= Registers::AF;
        else
            regs.FLAGS &= ~Registers::AF;

        updateFlags(destReg, false);
    }
}

void Emulator8086::mul(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("MUL requires 1 operand");

    uint32_t result;
    uint16_t multiplier;

    if (isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = parseMemoryOperand(operands[0]);
        uint16_t address = calculateEffectiveAddress(memOp);
        if (is8BitRegister(operands[0]))
        {
            multiplier = readMemoryByte(address);
        }
        else
        {
            multiplier = readMemoryWord(address);
        }
    }
    else
    {
        multiplier = getValue(operands[0]);
    }

    if (is8BitRegister(operands[0]))
    {
        result = static_cast<uint32_t>(regs.AX.l) * (multiplier & 0xFF);
        regs.AX.x = static_cast<uint16_t>(result & 0xFFFF);

        if (result > 0xFF)
        {
            regs.FLAGS |= Registers::CF | Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~(Registers::CF | Registers::OF);
        }
    }

    else
    {
        uint32_t ax = static_cast<uint32_t>(regs.AX.x);
        result = ax * multiplier;

        regs.DX.x = static_cast<uint16_t>((result >> 16) & 0xFFFF);
        regs.AX.x = static_cast<uint16_t>(result & 0xFFFF);

        if (regs.DX.x != 0)
        {
            regs.FLAGS |= Registers::CF | Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~(Registers::CF | Registers::OF);
        }
    }

    if (result == 0)
    {
        regs.FLAGS |= Registers::ZF;
    }
    else
    {
        regs.FLAGS &= ~Registers::ZF;
    }

    if (is8BitRegister(operands[0]))
    {
        if (result & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }
    }
    else
    {
        if (result & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }
    }

    uint8_t lowByte = result & 0xFF;
    bool parity = true;
    for (int i = 0; i < 8; i++)
    {
        if (lowByte & (1 << i))
        {
            parity = !parity;
        }
    }
    if (parity)
    {
        regs.FLAGS |= Registers::PF;
    }
    else
    {
        regs.FLAGS &= ~Registers::PF;
    }

    regs.FLAGS &= ~Registers::AF;
}

void Emulator8086::div(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("DIV requires 1 operand");

    uint32_t dividend;
    uint16_t divisor;

    if (isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = parseMemoryOperand(operands[0]);
        uint16_t address = calculateEffectiveAddress(memOp);
        if (is8BitRegister(operands[0]))
        {
            divisor = readMemoryByte(address);
        }
        else
        {
            divisor = readMemoryWord(address);
        }
    }
    else
    {
        divisor = getValue(operands[0]);
    }

    if (divisor == 0)
    {
        throw std::runtime_error("Division by zero");
    }

    if (is8BitRegister(operands[0]))
    {
        dividend = static_cast<uint32_t>(regs.AX.x);
        divisor &= 0xFF;

        uint16_t quotient = static_cast<uint16_t>(dividend / divisor);
        uint8_t remainder = static_cast<uint8_t>(dividend % divisor);

        if (quotient > 0xFF)
        {
            throw std::runtime_error("Division overflow");
        }

        regs.AX.l = static_cast<uint8_t>(quotient);
        regs.AX.h = remainder;
    }

    else
    {

        dividend = (static_cast<uint32_t>(regs.DX.x) << 16) | regs.AX.x;

        uint32_t quotient = dividend / divisor;
        uint16_t remainder = static_cast<uint16_t>(dividend % divisor);

        if (quotient > 0xFFFF)
        {
            throw std::runtime_error("Division overflow");
        }

        regs.AX.x = static_cast<uint16_t>(quotient);
        regs.DX.x = remainder;
    }

    regs.FLAGS &= ~(Registers::OF | Registers::CF);

    if (is8BitRegister(operands[0]))
    {
        if (regs.AX.l == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (regs.AX.l & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }
    }
    else
    {
        if (regs.AX.x == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (regs.AX.x & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }
    }

    uint8_t lowByte = regs.AX.l;
    bool parity = true;
    for (int i = 0; i < 8; i++)
    {
        if (lowByte & (1 << i))
        {
            parity = !parity;
        }
    }
    if (parity)
    {
        regs.FLAGS |= Registers::PF;
    }
    else
    {
        regs.FLAGS &= ~Registers::PF;
    }

    regs.FLAGS &= ~Registers::AF;
}

void Emulator8086::shl(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHL requires 2 operands");

    const std::string &dest = operands[0];
    uint8_t count;

    if (operands[1] == "CL")
    {
        count = regs.CX.l & 0x1F;
    }
    else
    {
        count = static_cast<uint8_t>(parseNumericValue(operands[1]) & 0x1F);
    }

    if (count == 0)
        return;

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t original = destReg;

        for (uint8_t i = 0; i < count; i++)
        {

            bool msb = (destReg & 0x80) != 0;

            bool lastBit = (destReg & 0x80) != 0;
            if (lastBit)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            destReg <<= 1;

            if (i == 0)
            {
                bool newMsb = (destReg & 0x80) != 0;
                if (msb != newMsb)
                {
                    regs.FLAGS |= Registers::OF;
                }
                else
                {
                    regs.FLAGS &= ~Registers::OF;
                }
            }
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parityByte = destReg;
        bool parity = true;
        for (int i = 0; i < 8; i++)
        {
            if (parityByte & (1 << i))
            {
                parity = !parity;
            }
        }
        if (parity)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }

    else
    {
        uint16_t value;
        uint16_t address = 0;

        if (isMemoryOperand(dest))
        {
            MemoryOperand memOp = parseMemoryOperand(dest);
            address = calculateEffectiveAddress(memOp);
            value = readMemoryWord(address);
        }
        else
        {
            value = getRegister(dest);
        }

        uint16_t original = value;

        for (uint8_t i = 0; i < count; i++)
        {

            bool msb = (value & 0x8000) != 0;

            bool lastBit = (value & 0x8000) != 0;
            if (lastBit)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            value <<= 1;

            if (i == 0)
            {
                bool newMsb = (value & 0x8000) != 0;
                if (msb != newMsb)
                {
                    regs.FLAGS |= Registers::OF;
                }
                else
                {
                    regs.FLAGS &= ~Registers::OF;
                }
            }
        }

        if (isMemoryOperand(dest))
        {
            writeMemoryWord(address, value);
        }
        else
        {
            getRegister(dest) = value;
        }

        if (value == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (value & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parityByte = value & 0xFF;
        bool parity = true;
        for (int i = 0; i < 8; i++)
        {
            if (parityByte & (1 << i))
            {
                parity = !parity;
            }
        }
        if (parity)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }

    regs.FLAGS &= ~Registers::AF;
}

void Emulator8086::shr(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHR requires 2 operands");

    const std::string &dest = operands[0];
    uint8_t count;

    if (operands[1] == "CL")
    {
        count = regs.CX.l & 0x1F;
    }
    else
    {
        count = static_cast<uint8_t>(parseNumericValue(operands[1]) & 0x1F);
    }

    if (count == 0)
        return;

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t original = destReg;

        for (uint8_t i = 0; i < count; i++)
        {

            bool msb = (destReg & 0x80) != 0;

            bool lastBit = (destReg & 0x01) != 0;
            if (lastBit)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            destReg >>= 1;

            if (i == 0)
            {
                if (msb)
                {
                    regs.FLAGS |= Registers::OF;
                }
                else
                {
                    regs.FLAGS &= ~Registers::OF;
                }
            }
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parityByte = destReg;
        bool parity = true;
        for (int i = 0; i < 8; i++)
        {
            if (parityByte & (1 << i))
            {
                parity = !parity;
            }
        }
        if (parity)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }

    else
    {
        uint16_t value;
        uint16_t address = 0;

        if (isMemoryOperand(dest))
        {
            MemoryOperand memOp = parseMemoryOperand(dest);
            address = calculateEffectiveAddress(memOp);
            value = readMemoryWord(address);
        }
        else
        {
            value = getRegister(dest);
        }

        uint16_t original = value;

        for (uint8_t i = 0; i < count; i++)
        {

            bool msb = (value & 0x8000) != 0;

            bool lastBit = (value & 0x0001) != 0;
            if (lastBit)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            value >>= 1;

            if (i == 0)
            {
                if (msb)
                {
                    regs.FLAGS |= Registers::OF;
                }
                else
                {
                    regs.FLAGS &= ~Registers::OF;
                }
            }
        }

        if (isMemoryOperand(dest))
        {
            writeMemoryWord(address, value);
        }
        else
        {
            getRegister(dest) = value;
        }

        if (value == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (value & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parityByte = value & 0xFF;
        bool parity = true;
        for (int i = 0; i < 8; i++)
        {
            if (parityByte & (1 << i))
            {
                parity = !parity;
            }
        }
        if (parity)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }

    regs.FLAGS &= ~Registers::AF;
}

void Emulator8086::push(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("PUSH requires 1 operand");

    const std::string &source = operands[0];
    uint16_t value;

    std::string upperSource = source;
    std::transform(upperSource.begin(), upperSource.end(), upperSource.begin(), ::toupper);
    if (isSegmentRegister(upperSource))
    {
        value = getSegmentRegister(upperSource);
    }

    else if (isMemoryOperand(source))
    {
        MemoryOperand memOp = parseMemoryOperand(source);
        uint16_t address = calculateEffectiveAddress(memOp);
        value = readMemoryWord(address);
    }

    else if (is8BitRegister(source))
    {
        throw std::runtime_error("Cannot push 8-bit register");
    }

    else if (registers.find(upperSource) != registers.end())
    {
        value = getRegister(upperSource);
    }

    else
    {
        value = parseNumericValue(source);
    }

    if (regs.SP <= 2)
    {
        throw std::runtime_error("Stack overflow");
    }

    regs.SP -= 2;

    writeMemoryWord(regs.SP, value);
}

void Emulator8086::pop(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("POP requires 1 operand");

    const std::string &dest = operands[0];

    if (regs.SP >= 0xFFFE)
    {
        throw std::runtime_error("Stack underflow");
    }

    uint16_t value = readMemoryWord(regs.SP);

    std::string upperDest = dest;
    std::transform(upperDest.begin(), upperDest.end(), upperDest.begin(), ::toupper);
    if (isSegmentRegister(upperDest))
    {

        if (upperDest == "CS")
        {
            throw std::runtime_error("Cannot POP to CS register");
        }
        setSegmentRegister(upperDest, value);
    }

    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);
        writeMemoryWord(address, value);
    }

    else if (is8BitRegister(dest))
    {
        throw std::runtime_error("Cannot POP to 8-bit register");
    }

    else if (registers.find(upperDest) != registers.end())
    {
        uint16_t &destReg = getRegister(upperDest);
        destReg = value;
    }
    else
    {
        throw std::runtime_error("Invalid POP destination operand");
    }

    regs.SP += 2;

    writeMemoryWord(regs.SP - 2, 0);
}

void Emulator8086::cmp(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("CMP requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t destValue = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            uint16_t value = parseNumericValue(source);
            if (value > 0xFF)
            {
                throw std::runtime_error("Value too large for 8-bit comparison");
            }
            sourceValue = static_cast<uint8_t>(value);
        }

        uint16_t result = static_cast<uint16_t>(destValue) - sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::ZF | Registers::SF | Registers::OF | Registers::AF | Registers::PF);

        if (result == 0)
            regs.FLAGS |= Registers::ZF;
        if (result & 0x80)
            regs.FLAGS |= Registers::SF;
        if (destValue < sourceValue)
            regs.FLAGS |= Registers::CF;
        if ((destValue ^ sourceValue) & (destValue ^ result) & 0x80)
            regs.FLAGS |= Registers::OF;
        if (((destValue & 0xF) - (sourceValue & 0xF)) & 0x10)
            regs.FLAGS |= Registers::AF;
        if (__builtin_parity(result & 0xFF))
            regs.FLAGS |= Registers::PF;
    }

    else
    {
        uint16_t destValue;
        uint16_t sourceValue;

        if (isMemoryOperand(dest))
        {
            MemoryOperand memOpDest = parseMemoryOperand(dest);
            uint16_t address = calculateEffectiveAddress(memOpDest);
            destValue = readMemoryWord(address);
        }
        else
        {
            destValue = getRegister(dest);
        }

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSource = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOpSource);
            sourceValue = readMemoryWord(address);
        }
        else if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint32_t result = static_cast<uint32_t>(destValue) - sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::ZF | Registers::SF | Registers::OF | Registers::AF | Registers::PF);

        if ((result & 0xFFFF) == 0)
            regs.FLAGS |= Registers::ZF;
        if (result & 0x8000)
            regs.FLAGS |= Registers::SF;
        if (destValue < sourceValue)
            regs.FLAGS |= Registers::CF;
        if ((destValue ^ sourceValue) & (destValue ^ result) & 0x8000)
            regs.FLAGS |= Registers::OF;
        if (((destValue & 0xF) - (sourceValue & 0xF)) & 0x10)
            regs.FLAGS |= Registers::AF;
        if (__builtin_parity(result & 0xFF))
            regs.FLAGS |= Registers::PF;
    }
}

void Emulator8086::test(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("TEST requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t destValue = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            uint16_t value = parseNumericValue(source);
            if (value > 0xFF)
            {
                throw std::runtime_error("Value too large for 8-bit TEST");
            }
            sourceValue = static_cast<uint8_t>(value);
        }

        uint8_t result = destValue & sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::ZF | Registers::SF | Registers::OF | Registers::PF);

        if (result == 0)
            regs.FLAGS |= Registers::ZF;
        if (result & 0x80)
            regs.FLAGS |= Registers::SF;
        if (__builtin_parity(result))
            regs.FLAGS |= Registers::PF;
    }

    else
    {
        uint16_t destValue;
        uint16_t sourceValue;

        if (isMemoryOperand(dest))
        {
            MemoryOperand memOpDest = parseMemoryOperand(dest);
            uint16_t address = calculateEffectiveAddress(memOpDest);
            destValue = readMemoryWord(address);
        }
        else if (registers.find(dest) != registers.end())
        {
            destValue = getRegister(dest);
        }
        else
        {
            throw std::runtime_error("Invalid destination operand for TEST");
        }

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSource = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOpSource);
            sourceValue = readMemoryWord(address);
        }
        else if (registers.find(source) != registers.end())
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t result = destValue & sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::ZF | Registers::SF | Registers::OF | Registers::PF);

        if (result == 0)
            regs.FLAGS |= Registers::ZF;
        if (result & 0x8000)
            regs.FLAGS |= Registers::SF;
        if (__builtin_parity(result & 0xFF))
            regs.FLAGS |= Registers::PF;
    }
}

void Emulator8086::jmp(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JMP requires 1 operand");

    const std::string &target = operands[0];
    uint16_t jumpTarget;

    if (isMemoryOperand(target))
    {
        MemoryOperand memOp = parseMemoryOperand(target);
        uint16_t address = calculateEffectiveAddress(memOp);
        jumpTarget = readMemoryWord(address);
    }

    else if (registers.find(target) != registers.end())
    {
        jumpTarget = getRegister(target);
    }

    else if (labels.find(target) != labels.end())
    {
        jumpTarget = labels[target];
    }

    else
    {
        jumpTarget = parseNumericValue(target);
        if (jumpTarget >= programSize)
        {
            throw std::runtime_error("Jump target out of program bounds");
        }
    }

    currentInstructionIndex = jumpTarget;
}

void Emulator8086::je(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JE/JZ requires 1 operand");
    if (regs.FLAGS & Registers::ZF)
    {
        jmp(operands);
    }
}

void Emulator8086::jne(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNE/JNZ requires 1 operand");
    if (!(regs.FLAGS & Registers::ZF))
    {
        jmp(operands);
    }
}

void Emulator8086::jl(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JL/JNGE requires 1 operand");
    bool SF = (regs.FLAGS & Registers::SF) != 0;
    bool OF = (regs.FLAGS & Registers::OF) != 0;
    if (SF != OF)
    {
        jmp(operands);
    }
}

void Emulator8086::jle(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JLE/JNG requires 1 operand");
    bool ZF = (regs.FLAGS & Registers::ZF) != 0;
    bool SF = (regs.FLAGS & Registers::SF) != 0;
    bool OF = (regs.FLAGS & Registers::OF) != 0;
    if (ZF || (SF != OF))
    {
        jmp(operands);
    }
}

void Emulator8086::jg(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JG/JNLE requires 1 operand");
    bool ZF = (regs.FLAGS & Registers::ZF) != 0;
    bool SF = (regs.FLAGS & Registers::SF) != 0;
    bool OF = (regs.FLAGS & Registers::OF) != 0;
    if (!ZF && (SF == OF))
    {
        jmp(operands);
    }
}

void Emulator8086::jge(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JGE/JNL requires 1 operand");
    bool SF = (regs.FLAGS & Registers::SF) != 0;
    bool OF = (regs.FLAGS & Registers::OF) != 0;
    if (SF == OF)
    {
        jmp(operands);
    }
}

void Emulator8086::ja(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JA/JNBE requires 1 operand");
    bool CF = (regs.FLAGS & Registers::CF) != 0;
    bool ZF = (regs.FLAGS & Registers::ZF) != 0;
    if (!CF && !ZF)
    {
        jmp(operands);
    }
}

void Emulator8086::jae(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JAE/JNB/JNC requires 1 operand");
    if (!(regs.FLAGS & Registers::CF))
    {
        jmp(operands);
    }
}

void Emulator8086::jb(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JB/JNAE/JC requires 1 operand");
    if (regs.FLAGS & Registers::CF)
    {
        jmp(operands);
    }
}

void Emulator8086::jbe(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JBE/JNA requires 1 operand");
    bool CF = (regs.FLAGS & Registers::CF) != 0;
    bool ZF = (regs.FLAGS & Registers::ZF) != 0;
    if (CF || ZF)
    {
        jmp(operands);
    }
}

void Emulator8086::inc(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("INC requires 1 operand");

    const std::string &dest = operands[0];

    bool oldCF = (regs.FLAGS & Registers::CF) != 0;

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t oldValue = destReg;
        destReg++;

        regs.FLAGS &= ~(Registers::ZF | Registers::SF | Registers::OF | Registers::AF | Registers::PF);

        if (destReg == 0)
            regs.FLAGS |= Registers::ZF;
        if (destReg & 0x80)
            regs.FLAGS |= Registers::SF;
        if (oldValue == 0x7F)
            regs.FLAGS |= Registers::OF;
        if ((oldValue & 0xF) == 0xF)
            regs.FLAGS |= Registers::AF;
        if (__builtin_parity(destReg))
            regs.FLAGS |= Registers::PF;
    }

    else
    {
        uint16_t value;
        uint16_t *destPtr = nullptr;

        if (isMemoryOperand(dest))
        {
            MemoryOperand memOp = parseMemoryOperand(dest);
            uint16_t address = calculateEffectiveAddress(memOp);
            value = readMemoryWord(address);
        }
        else if (registers.find(dest) != registers.end())
        {
            destPtr = &getRegister(dest);
            value = *destPtr;
        }
        else
        {
            throw std::runtime_error("Invalid operand for INC");
        }

        uint16_t oldValue = value;
        value++;

        if (destPtr)
        {
            *destPtr = value;
        }
        else
        {
            MemoryOperand memOp = parseMemoryOperand(dest);
            uint16_t address = calculateEffectiveAddress(memOp);
            writeMemoryWord(address, value);
        }

        regs.FLAGS &= ~(Registers::ZF | Registers::SF | Registers::OF | Registers::AF | Registers::PF);

        if (value == 0)
            regs.FLAGS |= Registers::ZF;
        if (value & 0x8000)
            regs.FLAGS |= Registers::SF;
        if (oldValue == 0x7FFF)
            regs.FLAGS |= Registers::OF;
        if ((oldValue & 0xF) == 0xF)
            regs.FLAGS |= Registers::AF;
        if (__builtin_parity(value & 0xFF))
            regs.FLAGS |= Registers::PF;
    }

    if (oldCF)
    {
        regs.FLAGS |= Registers::CF;
    }
    else
    {
        regs.FLAGS &= ~Registers::CF;
    }
}

void Emulator8086::dec(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("DEC requires 1 operand");

    const std::string &dest = operands[0];

    bool originalCarry = (regs.FLAGS & Registers::CF) != 0;

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t oldValue = destReg;
        destReg--;

        if ((oldValue & 0x0F) == 0)
        {
            regs.FLAGS |= Registers::AF;
        }
        else
        {
            regs.FLAGS &= ~Registers::AF;
        }

        if (oldValue == 0x80)
        {
            regs.FLAGS |= Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~Registers::OF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parity = destReg;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);
        uint16_t oldValue = readMemoryWord(address);
        uint16_t result = oldValue - 1;

        if ((oldValue & 0x0F) == 0)
        {
            regs.FLAGS |= Registers::AF;
        }
        else
        {
            regs.FLAGS &= ~Registers::AF;
        }

        if (oldValue == 0x8000)
        {
            regs.FLAGS |= Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~Registers::OF;
        }

        writeMemoryWord(address, result);

        if (result == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (result & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parity = result & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t oldValue = destReg;
        destReg--;

        if ((oldValue & 0x0F) == 0)
        {
            regs.FLAGS |= Registers::AF;
        }
        else
        {
            regs.FLAGS &= ~Registers::AF;
        }

        if (oldValue == 0x8000)
        {
            regs.FLAGS |= Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~Registers::OF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        if (destReg & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        uint8_t parity = destReg & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }

    if (originalCarry)
    {
        regs.FLAGS |= Registers::CF;
    }
    else
    {
        regs.FLAGS &= ~Registers::CF;
    }
}

void Emulator8086::and_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("AND requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            sourceValue = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }

        destReg &= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        uint16_t destValue = readMemoryWord(address);
        uint16_t sourceValue;

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t result = destValue & sourceValue;
        writeMemoryWord(address, result);

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (result & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (result == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = result & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t sourceValue;

        if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryWord(address);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        destReg &= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
}

void Emulator8086::or_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OR requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            sourceValue = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }

        destReg |= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        uint16_t destValue = readMemoryWord(address);
        uint16_t sourceValue;

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t result = destValue | sourceValue;
        writeMemoryWord(address, result);

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (result & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (result == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = result & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t sourceValue;

        if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryWord(address);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        destReg |= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
}

void Emulator8086::xor_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XOR requires 2 operands");

    const std::string &dest = operands[0];
    const std::string &source = operands[1];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        uint8_t sourceValue;

        if (is8BitRegister(source))
        {
            sourceValue = getRegister8(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryByte(address);
        }
        else
        {
            sourceValue = static_cast<uint8_t>(parseNumericValue(source) & 0xFF);
        }

        destReg ^= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOpDest = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOpDest);
        uint16_t destValue = readMemoryWord(address);
        uint16_t sourceValue;

        if (isMemoryOperand(source))
        {
            MemoryOperand memOpSrc = parseMemoryOperand(source);
            uint16_t srcAddress = calculateEffectiveAddress(memOpSrc);
            sourceValue = readMemoryWord(srcAddress);
        }
        else if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        uint16_t result = destValue ^ sourceValue;
        writeMemoryWord(address, result);

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (result & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (result == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = result & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else
    {
        uint16_t &destReg = getRegister(dest);
        uint16_t sourceValue;

        if (isRegister(source))
        {
            sourceValue = getRegister(source);
        }
        else if (isMemoryOperand(source))
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            sourceValue = readMemoryWord(address);
        }
        else
        {
            sourceValue = parseNumericValue(source);
        }

        destReg ^= sourceValue;

        regs.FLAGS &= ~(Registers::CF | Registers::OF);

        if (destReg & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
}

void Emulator8086::not_op(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("NOT requires 1 operand");

    const std::string &dest = operands[0];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);
        destReg = ~destReg;
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);

        if (memOp.size == 8)
        {

            uint8_t value = readMemoryByte(address);
            writeMemoryByte(address, ~value);
        }
        else
        {

            uint16_t value = readMemoryWord(address);
            writeMemoryWord(address, ~value);
        }
    }
    else if (is16BitRegister(dest))
    {
        uint16_t &destReg = getRegister(dest);
        destReg = ~destReg;
    }
    else
    {
        throw std::runtime_error("Invalid operand for NOT instruction");
    }
}

void Emulator8086::neg(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("NEG requires 1 operand");

    const std::string &dest = operands[0];

    if (is8BitRegister(dest))
    {
        uint8_t &destReg = getRegister8(dest);

        if (destReg != 0)
        {
            regs.FLAGS |= Registers::CF;
        }
        else
        {
            regs.FLAGS &= ~Registers::CF;
        }

        destReg = (~destReg) + 1;

        if (destReg == 0x80)
        {
            regs.FLAGS |= Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~Registers::OF;
        }

        if (destReg & 0x80)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);

        if (memOp.size == 8)
        {
            uint8_t value = readMemoryByte(address);

            if (value != 0)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            uint8_t result = (~value) + 1;
            writeMemoryByte(address, result);

            if (result == 0x80)
            {
                regs.FLAGS |= Registers::OF;
            }
            else
            {
                regs.FLAGS &= ~Registers::OF;
            }

            if (result & 0x80)
                regs.FLAGS |= Registers::SF;
            else
                regs.FLAGS &= ~Registers::SF;

            if (result == 0)
                regs.FLAGS |= Registers::ZF;
            else
                regs.FLAGS &= ~Registers::ZF;

            uint8_t parity = result;
            parity ^= parity >> 4;
            parity ^= parity >> 2;
            parity ^= parity >> 1;
            if (~parity & 1)
                regs.FLAGS |= Registers::PF;
            else
                regs.FLAGS &= ~Registers::PF;
        }
        else
        {
            uint16_t value = readMemoryWord(address);

            if (value != 0)
            {
                regs.FLAGS |= Registers::CF;
            }
            else
            {
                regs.FLAGS &= ~Registers::CF;
            }

            uint16_t result = (~value) + 1;
            writeMemoryWord(address, result);

            if (result == 0x8000)
            {
                regs.FLAGS |= Registers::OF;
            }
            else
            {
                regs.FLAGS &= ~Registers::OF;
            }

            if (result & 0x8000)
                regs.FLAGS |= Registers::SF;
            else
                regs.FLAGS &= ~Registers::SF;

            if (result == 0)
                regs.FLAGS |= Registers::ZF;
            else
                regs.FLAGS &= ~Registers::ZF;

            uint8_t parity = result & 0xFF;
            parity ^= parity >> 4;
            parity ^= parity >> 2;
            parity ^= parity >> 1;
            if (~parity & 1)
                regs.FLAGS |= Registers::PF;
            else
                regs.FLAGS &= ~Registers::PF;
        }
    }
    else if (is16BitRegister(dest))
    {
        uint16_t &destReg = getRegister(dest);

        if (destReg != 0)
        {
            regs.FLAGS |= Registers::CF;
        }
        else
        {
            regs.FLAGS &= ~Registers::CF;
        }

        destReg = (~destReg) + 1;

        if (destReg == 0x8000)
        {
            regs.FLAGS |= Registers::OF;
        }
        else
        {
            regs.FLAGS &= ~Registers::OF;
        }

        if (destReg & 0x8000)
        {
            regs.FLAGS |= Registers::SF;
        }
        else
        {
            regs.FLAGS &= ~Registers::SF;
        }

        if (destReg == 0)
        {
            regs.FLAGS |= Registers::ZF;
        }
        else
        {
            regs.FLAGS &= ~Registers::ZF;
        }

        uint8_t parity = destReg & 0xFF;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        if (~parity & 1)
        {
            regs.FLAGS |= Registers::PF;
        }
        else
        {
            regs.FLAGS &= ~Registers::PF;
        }
    }
    else
    {
        throw std::runtime_error("Invalid operand for NEG instruction");
    }
}

void Emulator8086::executeInstruction(const std::string &instruction)
{
    std::istringstream iss(instruction);
    std::string opcode;
    iss >> opcode;

    std::transform(opcode.begin(), opcode.end(), opcode.begin(), ::toupper);

    std::vector<std::string> operands;
    std::string operand;
    while (std::getline(iss >> std::ws, operand, ','))
    {

        operand.erase(0, operand.find_first_not_of(" \t"));
        operand.erase(operand.find_last_not_of(" \t") + 1);
        operands.push_back(operand);
    }

    auto it = instructions.find(opcode);
    if (it == instructions.end())
    {
        throw std::runtime_error("Unknown instruction: " + opcode);
    }

    it->second(operands);
}

std::string toBinaryString(uint16_t value)
{
    return std::bitset<16>(value).to_string();
}

void Emulator8086::displayRegisters()
{
    std::cout << std::hex << std::uppercase << std::setfill('0')
              << "AX=" << toBinaryString(regs.AX.x) << " (AH=" << toBinaryString(regs.AX.h)
              << ", AL=" << toBinaryString(regs.AX.l) << ")\n"
              << "BX=" << toBinaryString(regs.BX.x) << " (BH=" << toBinaryString(regs.BX.h)
              << ", BL=" << toBinaryString(regs.BX.l) << ")\n"
              << "CX=" << toBinaryString(regs.CX.x) << " (CH=" << toBinaryString(regs.CX.h)
              << ", CL=" << toBinaryString(regs.CX.l) << ")\n"
              << "DX=" << toBinaryString(regs.DX.x) << " (DH=" << toBinaryString(regs.DX.h)
              << ", DL=" << toBinaryString(regs.DX.l) << ")\n"
              << "SI=" << toBinaryString(regs.SI) << "  "
              << "DI=" << toBinaryString(regs.DI) << "  "
              << "BP=" << toBinaryString(regs.BP) << "  "
              << "SP=" << toBinaryString(regs.SP) << "\n"
              << "CS=" << toBinaryString(regs.CS) << "  "
              << "DS=" << toBinaryString(regs.DS) << "  "
              << "ES=" << toBinaryString(regs.ES) << "  "
              << "SS=" << toBinaryString(regs.SS) << "\n"
              << "IP=" << toBinaryString(regs.IP) << "  "
              << "FLAGS=" << std::bitset<16>(regs.FLAGS)
              << " [" << (regs.FLAGS & Registers::OF ? "O" : "-")
              << (regs.FLAGS & Registers::DF ? "D" : "-")
              << (regs.FLAGS & Registers::IF ? "I" : "-")
              << (regs.FLAGS & Registers::TF ? "T" : "-")
              << (regs.FLAGS & Registers::SF ? "S" : "-")
              << (regs.FLAGS & Registers::ZF ? "Z" : "-")
              << (regs.FLAGS & Registers::AF ? "A" : "-")
              << (regs.FLAGS & Registers::PF ? "P" : "-")
              << (regs.FLAGS & Registers::CF ? "C" : "-")
              << "]\n";
}

uint8_t Emulator8086::getValue8(const std::string &operand)
{
    if (operand[0] >= '0' && operand[0] <= '9')
    {
        return std::stoi(operand, nullptr, 16) & 0xFF;
    }
    return getRegister8(operand);
}

void Emulator8086::displayHelp()
{
    std::cout << "Supported Instructions:\n";
    std::cout << "1. ADD <operand1>, <operand2>  - Adds the two operands\n";
    std::cout << "2. SUB <operand1>, <operand2>  - Subtracts operand2 from operand1\n";
    std::cout << "3. PUSH <operand>              - Pushes the operand onto the stack\n";
    std::cout << "4. POP <operand>               - Pops the value from the stack into the operand\n";
    std::cout << "5. CMP <operand1>, <operand2>  - Compares two operands\n";
    std::cout << "6. JMP <label>                 - Jumps to the specified label\n";
    std::cout << "7. JE <label>                  - Jumps to the label if the zero flag is set\n";
    std::cout << "8. JNE <label>                 - Jumps to the label if the zero flag is not set\n";
    std::cout << "9. JG <label>                  - Jumps to the label if greater\n";
    std::cout << "10. JL <label>                 - Jumps to the label if less\n";
    std::cout << "11. INC <operand>              - Increments the operand by 1\n";
    std::cout << "12. DEC <operand>              - Decrements the operand by 1\n";
    std::cout << "13. AND <operand1>, <operand2> - Performs bitwise AND on the operands\n";
    std::cout << "14. OR <operand1>, <operand2>  - Performs bitwise OR on the operands\n";
    std::cout << "15. XOR <operand1>, <operand2> - Performs bitwise XOR on the operands\n";
    std::cout << "16. NOT <operand>              - Performs bitwise NOT on the operand\n";
    std::cout << "17. SHR <operand>, <count>     - Shifts operand right by count bits\n";
    std::cout << "18. SHL <operand>, <count>     - Shifts operand left by count bits\n";
    // std::cout << "19. ROR <operand>, <count>     - Rotates operand right by count bits\n";
    // std::cout << "20. ROL <operand>, <count>     - Rotates operand left by count bits\n";
    std::cout << "21. MUL <operand>              - Unsigned multiply AL/AX by operand\n";
    std::cout << "22. DIV <operand>              - Unsigned divide AX/DX:AX by operand\n";
    std::cout << "23. NEG <operand>              - Two's complement negation\n";
    // std::cout << "24. CALL <label>               - Calls procedure at label\n";
    // std::cout << "25. RET                        - Returns from procedure\n";
    // std::cout << "26. INT <interrupt_number>     - Calls interrupt handler\n";
}

void Emulator8086::displayMemory(uint16_t address, uint16_t count)
{
    std::cout << "\nMemory dump from " << std::hex << std::uppercase
              << std::setfill('0') << std::setw(4) << address << ":\n";

    for (uint16_t i = 0; i < count; i++)
    {
        if (i % 16 == 0)
        {
            std::cout << std::setw(4) << (address + i) << ": ";
        }

        std::cout << std::setw(2) << static_cast<int>(memory[address + i]) << ' ';

        if (i % 16 == 15 || i == count - 1)
        {
            std::cout << '\n';
        }
    }
}

void Emulator8086::displayStack()
{
    std::cout << "\nStack contents:\n";
    for (uint16_t i = regs.SP; i < 0xFFFE; i += 2)
    {
        std::cout << std::hex << std::uppercase << std::setfill('0')
                  << "SP+" << std::setw(4) << (i - regs.SP) << ": "
                  << std::setw(4) << readMemoryWord(i) << '\n';
    }
}