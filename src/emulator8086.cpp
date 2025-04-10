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
    instructions["PUSH"] = std::bind(&Emulator8086::push, this, std::placeholders::_1);
    instructions["POP"] = std::bind(&Emulator8086::pop, this, std::placeholders::_1);
    instructions["XCHG"] = std::bind(&Emulator8086::xchg, this, std::placeholders::_1);
    instructions["LEA"] = std::bind(&Emulator8086::lea, this, std::placeholders::_1);
    instructions["LDS"] = std::bind(&Emulator8086::lds, this, std::placeholders::_1);
    instructions["LES"] = std::bind(&Emulator8086::les, this, std::placeholders::_1);
    instructions["LAHF"] = std::bind(&Emulator8086::lahf, this, std::placeholders::_1);
    instructions["SAHF"] = std::bind(&Emulator8086::sahf, this, std::placeholders::_1);
    instructions["PUSHF"] = std::bind(&Emulator8086::pushf, this, std::placeholders::_1);
    instructions["POPF"] = std::bind(&Emulator8086::popf, this, std::placeholders::_1);
    instructions["PUSHA"] = std::bind(&Emulator8086::pusha, this, std::placeholders::_1);
    instructions["POPA"] = std::bind(&Emulator8086::popa, this, std::placeholders::_1);
    instructions["ADD"] = std::bind(&Emulator8086::add, this, std::placeholders::_1);
    instructions["ADC"] = std::bind(&Emulator8086::adc, this, std::placeholders::_1);
    instructions["INC"] = std::bind(&Emulator8086::inc, this, std::placeholders::_1);
    instructions["AAA"] = std::bind(&Emulator8086::aaa, this, std::placeholders::_1);
    instructions["DAA"] = std::bind(&Emulator8086::daa, this, std::placeholders::_1);
    instructions["SUB"] = std::bind(&Emulator8086::sub, this, std::placeholders::_1);
    instructions["SBB"] = std::bind(&Emulator8086::sbb, this, std::placeholders::_1);
    instructions["DEC"] = std::bind(&Emulator8086::dec, this, std::placeholders::_1);
    instructions["NEG"] = std::bind(&Emulator8086::neg, this, std::placeholders::_1);
    instructions["AAS"] = std::bind(&Emulator8086::aas, this, std::placeholders::_1);
    instructions["DAS"] = std::bind(&Emulator8086::das, this, std::placeholders::_1);
    instructions["MUL"] = std::bind(&Emulator8086::mul, this, std::placeholders::_1);
    instructions["IMUL"] = std::bind(&Emulator8086::imul, this, std::placeholders::_1);
    instructions["AAM"] = std::bind(&Emulator8086::aam, this, std::placeholders::_1);
    instructions["DIV"] = std::bind(&Emulator8086::div, this, std::placeholders::_1);
    instructions["IDIV"] = std::bind(&Emulator8086::idiv, this, std::placeholders::_1);
    instructions["AAD"] = std::bind(&Emulator8086::aad, this, std::placeholders::_1);
    instructions["CBW"] = std::bind(&Emulator8086::cbw, this, std::placeholders::_1);
    instructions["CWD"] = std::bind(&Emulator8086::cwd, this, std::placeholders::_1);
    instructions["AND"] = std::bind(&Emulator8086::and_op, this, std::placeholders::_1);
    instructions["OR"] = std::bind(&Emulator8086::or_op, this, std::placeholders::_1);
    instructions["XOR"] = std::bind(&Emulator8086::xor_op, this, std::placeholders::_1);
    instructions["NOT"] = std::bind(&Emulator8086::not_op, this, std::placeholders::_1);
    instructions["TEST"] = std::bind(&Emulator8086::test, this, std::placeholders::_1);
    instructions["CMP"] = std::bind(&Emulator8086::cmp, this, std::placeholders::_1);
    instructions["MOVSB"] = std::bind(&Emulator8086::movsb, this, std::placeholders::_1);
    instructions["MOVSW"] = std::bind(&Emulator8086::movsw, this, std::placeholders::_1);
    instructions["CMPSB"] = std::bind(&Emulator8086::cmpsb, this, std::placeholders::_1);
    instructions["CMPSW"] = std::bind(&Emulator8086::cmpsw, this, std::placeholders::_1);
    instructions["SCASB"] = std::bind(&Emulator8086::scasb, this, std::placeholders::_1);
    instructions["SCASW"] = std::bind(&Emulator8086::scasw, this, std::placeholders::_1);
    instructions["LODSB"] = std::bind(&Emulator8086::lodsb, this, std::placeholders::_1);
    instructions["LODSW"] = std::bind(&Emulator8086::lodsw, this, std::placeholders::_1);
    instructions["STOSB"] = std::bind(&Emulator8086::stosb, this, std::placeholders::_1);
    instructions["STOSW"] = std::bind(&Emulator8086::stosw, this, std::placeholders::_1);
    instructions["CALL"] = std::bind(&Emulator8086::call, this, std::placeholders::_1);
    instructions["JMP"] = std::bind(&Emulator8086::jmp, this, std::placeholders::_1);
    instructions["RET"] = std::bind(&Emulator8086::ret, this, std::placeholders::_1);
    instructions["RETF"] = std::bind(&Emulator8086::retf, this, std::placeholders::_1);
    instructions["JE"] = std::bind(&Emulator8086::je, this, std::placeholders::_1);
    instructions["JL"] = std::bind(&Emulator8086::jl, this, std::placeholders::_1);
    instructions["JLE"] = std::bind(&Emulator8086::jle, this, std::placeholders::_1);
    instructions["JB"] = std::bind(&Emulator8086::jb, this, std::placeholders::_1);
    instructions["JBE"] = std::bind(&Emulator8086::jbe, this, std::placeholders::_1);
    instructions["JP"] = std::bind(&Emulator8086::jp, this, std::placeholders::_1);
    instructions["JO"] = std::bind(&Emulator8086::jo, this, std::placeholders::_1);
    instructions["JS"] = std::bind(&Emulator8086::js, this, std::placeholders::_1);
    instructions["JNE"] = std::bind(&Emulator8086::jne, this, std::placeholders::_1);
    instructions["JNL"] = std::bind(&Emulator8086::jnl, this, std::placeholders::_1);
    instructions["JG"] = std::bind(&Emulator8086::jg, this, std::placeholders::_1);
    instructions["JNB"] = std::bind(&Emulator8086::jnb, this, std::placeholders::_1);
    instructions["JA"] = std::bind(&Emulator8086::ja, this, std::placeholders::_1);
    instructions["JNP"] = std::bind(&Emulator8086::jnp, this, std::placeholders::_1);
    instructions["JNO"] = std::bind(&Emulator8086::jno, this, std::placeholders::_1);
    instructions["JNS"] = std::bind(&Emulator8086::jns, this, std::placeholders::_1);
    instructions["LOOP"] = std::bind(&Emulator8086::loop, this, std::placeholders::_1);
    instructions["LOOPZ"] = std::bind(&Emulator8086::loopz, this, std::placeholders::_1);
    instructions["LOOPNZ"] = std::bind(&Emulator8086::loopnz, this, std::placeholders::_1);
    instructions["JCXZ"] = std::bind(&Emulator8086::jcxz, this, std::placeholders::_1);
    instructions["INT"] = std::bind(&Emulator8086::int_op, this, std::placeholders::_1);
    instructions["INTO"] = std::bind(&Emulator8086::into, this, std::placeholders::_1);
    instructions["IRET"] = std::bind(&Emulator8086::iret, this, std::placeholders::_1);
    instructions["RCL"] = std::bind(&Emulator8086::rcl, this, std::placeholders::_1);
    instructions["RCR"] = std::bind(&Emulator8086::rcr, this, std::placeholders::_1);
    instructions["ROL"] = std::bind(&Emulator8086::rol, this, std::placeholders::_1);
    instructions["ROR"] = std::bind(&Emulator8086::ror, this, std::placeholders::_1);
    instructions["SAL"] = std::bind(&Emulator8086::sal, this, std::placeholders::_1);
    instructions["SAR"] = std::bind(&Emulator8086::sar, this, std::placeholders::_1);
    instructions["SHL"] = std::bind(&Emulator8086::shl, this, std::placeholders::_1);
    instructions["SHR"] = std::bind(&Emulator8086::shr, this, std::placeholders::_1);
    instructions["CLC"] = std::bind(&Emulator8086::clc, this, std::placeholders::_1);
    instructions["CMC"] = std::bind(&Emulator8086::cmc, this, std::placeholders::_1);
    instructions["STC"] = std::bind(&Emulator8086::stc, this, std::placeholders::_1);
    instructions["CLD"] = std::bind(&Emulator8086::cld, this, std::placeholders::_1);
    instructions["STD"] = std::bind(&Emulator8086::std, this, std::placeholders::_1);
    instructions["CLI"] = std::bind(&Emulator8086::cli, this, std::placeholders::_1);
    instructions["STI"] = std::bind(&Emulator8086::sti, this, std::placeholders::_1);
    instructions["HLT"] = std::bind(&Emulator8086::hlt, this, std::placeholders::_1);
    instructions["WAIT"] = std::bind(&Emulator8086::wait, this, std::placeholders::_1);
    instructions["ESC"] = std::bind(&Emulator8086::esc, this, std::placeholders::_1);
    instructions["LOCK"] = std::bind(&Emulator8086::lock, this, std::placeholders::_1);
    instructions["NOP"] = std::bind(&Emulator8086::nop, this, std::placeholders::_1);
    instructions["IN"] = std::bind(&Emulator8086::in_op, this, std::placeholders::_1);
    instructions["OUT"] = std::bind(&Emulator8086::out, this, std::placeholders::_1);
    instructions["REP"] = std::bind(&Emulator8086::rep, this, std::placeholders::_1);
    instructions["REPE"] = std::bind(&Emulator8086::repe, this, std::placeholders::_1);
    instructions["REPNE"] = std::bind(&Emulator8086::repne, this, std::placeholders::_1);
    instructions["REPNZ"] = std::bind(&Emulator8086::repnz, this, std::placeholders::_1);
    instructions["REPZ"] = std::bind(&Emulator8086::repz, this, std::placeholders::_1);
    instructions["XLAT"] = std::bind(&Emulator8086::xlat, this, std::placeholders::_1);
    instructions["XLATB"] = std::bind(&Emulator8086::xlatb, this, std::placeholders::_1);
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

bool Emulator8086::isMemoryOperand(const std::string &operand)
{
    return operand[0] == '[' && operand.back() == ']';
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
        parts.push_back(current);

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
        ea += memOp.base;
    if (memOp.hasIndex)
        ea += memOp.index;
    if (memOp.hasDisplacement)
        ea += memOp.displacement;
    return ea;
}

uint16_t Emulator8086::readMemoryWord(uint16_t address)
{
    if (address + 1 >= memory.size())
        throw std::out_of_range("Memory address out of range");
    return (memory[address + 1] << 8) | memory[address];
}

void Emulator8086::writeMemoryWord(uint16_t address, uint16_t value)
{
    if (address + 1 >= memory.size())
        throw std::out_of_range("Memory address out of range");
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

uint8_t Emulator8086::readMemoryByte(uint16_t address)
{
    if (address >= memory.size())
        throw std::out_of_range("Memory address out of range");
    return memory[address];
}

void Emulator8086::writeMemoryByte(uint16_t address, uint8_t value)
{
    if (address >= memory.size())
        throw std::out_of_range("Memory address out of range");
    memory[address] = value;
}

uint16_t Emulator8086::getValue(const std::string &operand)
{
    if (isMemoryOperand(operand))
    {
        MemoryOperand memOp = parseMemoryOperand(operand);
        uint16_t address = calculateEffectiveAddress(memOp);
        return readMemoryWord(address);
    }
    else if (operand.back() == 'h')
    {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16);
    }
    else if (operand[0] >= '0' && operand[0] <= '9')
    {
        return std::stoi(operand);
    }
    else if (is8BitRegister(operand))
    {
        throw std::runtime_error("Cannot get 16-bit value from 8-bit register");
    }
    else
    {
        return getRegister(operand);
    }
}

uint8_t Emulator8086::getValue8(const std::string &operand)
{
    if (isMemoryOperand(operand))
    {
        MemoryOperand memOp = parseMemoryOperand(operand);
        uint16_t address = calculateEffectiveAddress(memOp);
        return readMemoryByte(address);
    }
    else if (operand.back() == 'h')
    {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16) & 0xFF;
    }
    else if (operand[0] >= '0' && operand[0] <= '9')
    {
        return std::stoi(operand) & 0xFF;
    }
    else if (is8BitRegister(operand))
    {
        return getRegister8(operand);
    }
    else
    {
        throw std::runtime_error("Invalid operand for 8-bit value");
    }
}

void Emulator8086::updateFlags(uint32_t result, bool isByte, bool checkCarry)
{
    uint16_t mask = isByte ? 0xFF : 0xFFFF;
    uint16_t res = result & mask;

    regs.FLAGS = (res == 0) ? (regs.FLAGS | Registers::ZF) : (regs.FLAGS & ~Registers::ZF);

    regs.FLAGS = (isByte ? (res & 0x80) : (res & 0x8000)) ? (regs.FLAGS | Registers::SF) : (regs.FLAGS & ~Registers::SF);

    if (checkCarry)
    {
        regs.FLAGS = (isByte ? (result > 0xFF) : (result > 0xFFFF)) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
    }

    uint8_t leastByte = res & 0xFF;
    uint8_t parity = 0;
    for (int i = 0; i < 8; ++i)
        parity ^= (leastByte >> i) & 1;
    regs.FLAGS = (parity == 0) ? (regs.FLAGS | Registers::PF) : (regs.FLAGS & ~Registers::PF);
}

void Emulator8086::mov(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("MOV requires 2 operands");
    const std::string &dest = operands[0];
    const std::string &src = operands[1];

    if (is8BitRegister(dest))
    {
        getRegister8(dest) = getValue8(src);
    }
    else if (isMemoryOperand(dest))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);
        if (is8BitRegister(src))
            writeMemoryByte(address, getRegister8(src));
        else
            writeMemoryWord(address, getValue(src));
    }
    else
    {
        getRegister(dest) = getValue(src);
    }
}

void Emulator8086::push(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("PUSH requires 1 operand");
    if (is8BitRegister(operands[0]))
        throw std::runtime_error("PUSH requires 16-bit operand");
    uint16_t value = getValue(operands[0]);
    regs.SP -= 2;
    writeMemoryWord(regs.SP, value);
}

void Emulator8086::pop(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("POP requires 1 operand");
    if (is8BitRegister(operands[0]))
        throw std::runtime_error("POP requires 16-bit operand");
    uint16_t value = readMemoryWord(regs.SP);
    regs.SP += 2;
    if (isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = parseMemoryOperand(operands[0]);
        writeMemoryWord(calculateEffectiveAddress(memOp), value);
    }
    else
    {
        getRegister(operands[0]) = value;
    }
}

void Emulator8086::xchg(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XCHG requires 2 operands");
    if (is8BitRegister(operands[0]) && is8BitRegister(operands[1]))
    {
        uint8_t temp = getRegister8(operands[0]);
        getRegister8(operands[0]) = getRegister8(operands[1]);
        getRegister8(operands[1]) = temp;
    }
    else if (!isMemoryOperand(operands[0]) && !isMemoryOperand(operands[1]))
    {
        uint16_t temp = getRegister(operands[0]);
        getRegister(operands[0]) = getRegister(operands[1]);
        getRegister(operands[1]) = temp;
    }
    else
    {
        throw std::runtime_error("XCHG not implemented for memory operands");
    }
}

void Emulator8086::lea(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LEA requires 2 operands");
    if (!isMemoryOperand(operands[1]))
        throw std::runtime_error("LEA requires memory source");
    MemoryOperand memOp = parseMemoryOperand(operands[1]);
    getRegister(operands[0]) = calculateEffectiveAddress(memOp);
}

void Emulator8086::lds(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LDS requires 2 operands");
    if (!isMemoryOperand(operands[1]))
        throw std::runtime_error("LDS requires memory source");
    MemoryOperand memOp = parseMemoryOperand(operands[1]);
    uint16_t address = calculateEffectiveAddress(memOp);
    getRegister(operands[0]) = readMemoryWord(address);
    regs.DS = readMemoryWord(address + 2);
}

void Emulator8086::les(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("LES requires 2 operands");
    if (!isMemoryOperand(operands[1]))
        throw std::runtime_error("LES requires memory source");
    MemoryOperand memOp = parseMemoryOperand(operands[1]);
    uint16_t address = calculateEffectiveAddress(memOp);
    getRegister(operands[0]) = readMemoryWord(address);
    regs.ES = readMemoryWord(address + 2);
}

void Emulator8086::lahf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LAHF takes no operands");
    regs.AX.h = regs.FLAGS & 0xFF;
}

void Emulator8086::sahf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SAHF takes no operands");
    regs.FLAGS = (regs.FLAGS & 0xFF00) | (regs.AX.h & 0xFF);
}

void Emulator8086::pushf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("PUSHF takes no operands");
    regs.SP -= 2;
    writeMemoryWord(regs.SP, regs.FLAGS);
}

void Emulator8086::popf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("POPF takes no operands");
    regs.FLAGS = readMemoryWord(regs.SP);
    regs.SP += 2;
}

void Emulator8086::pusha(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("PUSHA takes no operands");
    uint16_t tempSP = regs.SP;
    push({"AX"});
    push({"CX"});
    push({"DX"});
    push({"BX"});
    regs.SP -= 2;
    writeMemoryWord(regs.SP, tempSP);
    push({"BP"});
    push({"SI"});
    push({"DI"});
}

void Emulator8086::popa(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("POPA takes no operands");
    pop({"DI"});
    pop({"SI"});
    pop({"BP"});
    regs.SP += 2;
    pop({"BX"});
    pop({"DX"});
    pop({"CX"});
    pop({"AX"});
}

void Emulator8086::add(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ADD requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint16_t result = dest + src;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else if (isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = parseMemoryOperand(operands[0]);
        uint16_t address = calculateEffectiveAddress(memOp);
        if (is8BitRegister(operands[1]))
        {
            uint8_t destVal = readMemoryByte(address);
            uint8_t src = getRegister8(operands[1]);
            uint16_t result = destVal + src;
            writeMemoryByte(address, result & 0xFF);
            updateFlags(result, true, true);
        }
        else
        {
            uint16_t destVal = readMemoryWord(address);
            uint16_t src = getValue(operands[1]);
            uint32_t result = destVal + src;
            writeMemoryWord(address, result & 0xFFFF);
            updateFlags(result, false, true);
        }
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint32_t result = dest + src;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::adc(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ADC requires 2 operands");
    bool cf = regs.FLAGS & Registers::CF;
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint16_t result = dest + src + cf;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint32_t result = dest + src + cf;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::inc(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("INC requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint16_t result = dest + 1;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest + 1;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::aaa(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("AAA takes no operands");
    if ((regs.AX.l & 0x0F) > 9 || (regs.FLAGS & Registers::AF))
    {
        regs.AX.l += 6;
        regs.AX.h += 1;
        regs.FLAGS |= (Registers::AF | Registers::CF);
    }
    else
    {
        regs.FLAGS &= ~(Registers::AF | Registers::CF);
    }
    regs.AX.l &= 0x0F;
}

void Emulator8086::daa(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("DAA takes no operands");
    uint8_t oldAL = regs.AX.l;
    bool oldCF = regs.FLAGS & Registers::CF;
    regs.FLAGS &= ~Registers::CF;
    if ((regs.AX.l & 0x0F) > 9 || (regs.FLAGS & Registers::AF))
    {
        regs.AX.l += 6;
        regs.FLAGS |= Registers::AF;
    }
    if ((oldAL > 0x99) || oldCF)
    {
        regs.AX.l += 0x60;
        regs.FLAGS |= Registers::CF;
    }
}

void Emulator8086::sub(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SUB requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint16_t result = dest - src;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else if (isMemoryOperand(operands[0]))
    {
        MemoryOperand memOp = parseMemoryOperand(operands[0]);
        uint16_t address = calculateEffectiveAddress(memOp);
        if (is8BitRegister(operands[1]))
        {
            uint8_t destVal = readMemoryByte(address);
            uint8_t src = getRegister8(operands[1]);
            uint16_t result = destVal - src;
            writeMemoryByte(address, result & 0xFF);
            updateFlags(result, true, true);
        }
        else
        {
            uint16_t destVal = readMemoryWord(address);
            uint16_t src = getValue(operands[1]);
            uint32_t result = destVal - src;
            writeMemoryWord(address, result & 0xFFFF);
            updateFlags(result, false, true);
        }
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint32_t result = dest - src;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::sbb(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SBB requires 2 operands");
    bool cf = regs.FLAGS & Registers::CF;
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint16_t result = dest - src - cf;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint32_t result = dest - src - cf;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::dec(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("DEC requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint16_t result = dest - 1;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest - 1;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::neg(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("NEG requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint16_t result = 0 - dest;
        dest = result & 0xFF;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = 0 - dest;
        dest = result & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::aas(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("AAS takes no operands");
    if ((regs.AX.l & 0x0F) > 9 || (regs.FLAGS & Registers::AF))
    {
        regs.AX.l -= 6;
        regs.AX.h -= 1;
        regs.FLAGS |= (Registers::AF | Registers::CF);
    }
    else
    {
        regs.FLAGS &= ~(Registers::AF | Registers::CF);
    }
    regs.AX.l &= 0x0F;
}

void Emulator8086::das(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("DAS takes no operands");
    uint8_t oldAL = regs.AX.l;
    bool oldCF = regs.FLAGS & Registers::CF;
    regs.FLAGS &= ~Registers::CF;
    if ((regs.AX.l & 0x0F) > 9 || (regs.FLAGS & Registers::AF))
    {
        regs.AX.l -= 6;
        regs.FLAGS |= Registers::AF;
    }
    if ((oldAL > 0x99) || oldCF)
    {
        regs.AX.l -= 0x60;
        regs.FLAGS |= Registers::CF;
    }
}

void Emulator8086::mul(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("MUL requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint16_t result = regs.AX.l * getRegister8(operands[0]);
        regs.AX.x = result;
        updateFlags(result, false, true);
    }
    else
    {
        uint32_t result = regs.AX.x * getValue(operands[0]);
        regs.AX.x = result & 0xFFFF;
        regs.DX.x = (result >> 16) & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::imul(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("IMUL requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        int16_t result = static_cast<int8_t>(regs.AX.l) * static_cast<int8_t>(getRegister8(operands[0]));
        regs.AX.x = result;
        updateFlags(result, false, true);
    }
    else
    {
        int32_t result = static_cast<int16_t>(regs.AX.x) * static_cast<int16_t>(getValue(operands[0]));
        regs.AX.x = result & 0xFFFF;
        regs.DX.x = (result >> 16) & 0xFFFF;
        updateFlags(result, false, true);
    }
}

void Emulator8086::aam(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("AAM takes no operands");
    uint8_t al = regs.AX.l;
    regs.AX.h = al / 10;
    regs.AX.l = al % 10;
    updateFlags(regs.AX.x, false, false);
}

void Emulator8086::div(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("DIV requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint8_t divisor = getRegister8(operands[0]);
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        uint16_t dividend = regs.AX.x;
        regs.AX.l = dividend / divisor;
        regs.AX.h = dividend % divisor;
    }
    else
    {
        uint16_t divisor = getValue(operands[0]);
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        uint32_t dividend = (regs.DX.x << 16) | regs.AX.x;
        regs.AX.x = dividend / divisor;
        regs.DX.x = dividend % divisor;
    }
}

void Emulator8086::idiv(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("IDIV requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        int8_t divisor = static_cast<int8_t>(getRegister8(operands[0]));
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        int16_t dividend = static_cast<int16_t>(regs.AX.x);
        regs.AX.l = dividend / divisor;
        regs.AX.h = dividend % divisor;
    }
    else
    {
        int16_t divisor = static_cast<int16_t>(getValue(operands[0]));
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        int32_t dividend = (regs.DX.x << 16) | regs.AX.x;
        regs.AX.x = dividend / divisor;
        regs.DX.x = dividend % divisor;
    }
}

void Emulator8086::aad(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("AAD takes no operands");
    regs.AX.l = (regs.AX.h * 10) + regs.AX.l;
    regs.AX.h = 0;
    updateFlags(regs.AX.x, false, false);
}

void Emulator8086::cbw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CBW takes no operands");
    regs.AX.h = (regs.AX.l & 0x80) ? 0xFF : 0x00;
}

void Emulator8086::cwd(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CWD takes no operands");
    regs.DX.x = (regs.AX.x & 0x8000) ? 0xFFFF : 0x0000;
}

void Emulator8086::and_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("AND requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        dest &= src;
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        dest &= src;
        updateFlags(dest, false, false);
    }
}

void Emulator8086::or_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OR requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        dest |= src;
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        dest |= src;
        updateFlags(dest, false, false);
    }
}

void Emulator8086::xor_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XOR requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        dest ^= src;
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        dest ^= src;
        updateFlags(dest, false, false);
    }
}

void Emulator8086::not_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("NOT requires 1 operand");
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        dest = ~dest;
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        dest = ~dest;
    }
}

void Emulator8086::test(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("TEST requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint8_t result = dest & src;
        updateFlags(result, true, false);
    }
    else
    {
        uint16_t dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint16_t result = dest & src;
        updateFlags(result, false, false);
    }
}

void Emulator8086::cmp(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("CMP requires 2 operands");
    if (is8BitRegister(operands[0]))
    {
        uint8_t dest = getRegister8(operands[0]);
        uint8_t src = getValue8(operands[1]);
        uint16_t result = dest - src;
        updateFlags(result, true, true);
    }
    else
    {
        uint16_t dest = getRegister(operands[0]);
        uint16_t src = getValue(operands[1]);
        uint32_t result = dest - src;
        updateFlags(result, false, true);
    }
}

void Emulator8086::movsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("MOVSB takes no operands");
    uint8_t value = readMemoryByte(regs.SI);
    writeMemoryByte(regs.DI, value);
    int adjust = (regs.FLAGS & Registers::DF) ? -1 : 1;
    regs.SI += adjust;
    regs.DI += adjust;
}

void Emulator8086::movsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("MOVSW takes no operands");
    uint16_t value = readMemoryWord(regs.SI);
    writeMemoryWord(regs.DI, value);
    int adjust = (regs.FLAGS & Registers::DF) ? -2 : 2;
    regs.SI += adjust;
    regs.DI += adjust;
}

void Emulator8086::cmpsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMPSB takes no operands");
    uint8_t src = readMemoryByte(regs.SI);
    uint8_t dest = readMemoryByte(regs.DI);
    uint16_t result = src - dest;
    updateFlags(result, true, true);
    int adjust = (regs.FLAGS & Registers::DF) ? -1 : 1;
    regs.SI += adjust;
    regs.DI += adjust;
}

void Emulator8086::cmpsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMPSW takes no operands");
    uint16_t src = readMemoryWord(regs.SI);
    uint16_t dest = readMemoryWord(regs.DI);
    uint32_t result = src - dest;
    updateFlags(result, false, true);
    int adjust = (regs.FLAGS & Registers::DF) ? -2 : 2;
    regs.SI += adjust;
    regs.DI += adjust;
}

void Emulator8086::scasb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SCASB takes no operands");
    uint8_t dest = regs.AX.l;
    uint8_t src = readMemoryByte(regs.DI);
    uint16_t result = dest - src;
    updateFlags(result, true, true);
    int adjust = (regs.FLAGS & Registers::DF) ? -1 : 1;
    regs.DI += adjust;
}

void Emulator8086::scasw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SCASW takes no operands");
    uint16_t dest = regs.AX.x;
    uint16_t src = readMemoryWord(regs.DI);
    uint32_t result = dest - src;
    updateFlags(result, false, true);
    int adjust = (regs.FLAGS & Registers::DF) ? -2 : 2;
    regs.DI += adjust;
}

void Emulator8086::lodsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LODSB takes no operands");
    regs.AX.l = readMemoryByte(regs.SI);
    int adjust = (regs.FLAGS & Registers::DF) ? -1 : 1;
    regs.SI += adjust;
}

void Emulator8086::lodsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LODSW takes no operands");
    regs.AX.x = readMemoryWord(regs.SI);
    int adjust = (regs.FLAGS & Registers::DF) ? -2 : 2;
    regs.SI += adjust;
}

void Emulator8086::stosb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STOSB takes no operands");
    writeMemoryByte(regs.DI, regs.AX.l);
    int adjust = (regs.FLAGS & Registers::DF) ? -1 : 1;
    regs.DI += adjust;
}

void Emulator8086::stosw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STOSW takes no operands");
    writeMemoryWord(regs.DI, regs.AX.x);
    int adjust = (regs.FLAGS & Registers::DF) ? -2 : 2;
    regs.DI += adjust;
}

void Emulator8086::call(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("CALL requires 1 operand");
    regs.SP -= 2;
    writeMemoryWord(regs.SP, regs.IP);
    regs.IP = getValue(operands[0]);
}

void Emulator8086::jmp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JMP requires 1 operand");
    regs.IP = getValue(operands[0]);
}

void Emulator8086::ret(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("RET takes no operands");
    regs.IP = readMemoryWord(regs.SP);
    regs.SP += 2;
}

void Emulator8086::retf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("RETF takes no operands");
    regs.IP = readMemoryWord(regs.SP);
    regs.SP += 2;
    regs.CS = readMemoryWord(regs.SP);
    regs.SP += 2;
}

void Emulator8086::je(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JE requires 1 operand");
    if (regs.FLAGS & Registers::ZF)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jl(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JL requires 1 operand");
    if ((regs.FLAGS & Registers::SF) != (regs.FLAGS & Registers::OF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jle(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JLE requires 1 operand");
    if ((regs.FLAGS & Registers::ZF) || ((regs.FLAGS & Registers::SF) != (regs.FLAGS & Registers::OF)))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jb(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JB requires 1 operand");
    if (regs.FLAGS & Registers::CF)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jbe(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JBE requires 1 operand");
    if (regs.FLAGS & (Registers::CF | Registers::ZF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JP requires 1 operand");
    if (regs.FLAGS & Registers::PF)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jo(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JO requires 1 operand");
    if (regs.FLAGS & Registers::OF)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::js(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JS requires 1 operand");
    if (regs.FLAGS & Registers::SF)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jne(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNE requires 1 operand");
    if (!(regs.FLAGS & Registers::ZF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jnl(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNL requires 1 operand");
    if ((regs.FLAGS & Registers::SF) == (regs.FLAGS & Registers::OF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jg(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JG requires 1 operand");
    if (!(regs.FLAGS & Registers::ZF) && ((regs.FLAGS & Registers::SF) == (regs.FLAGS & Registers::OF)))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jnb(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNB requires 1 operand");
    if (!(regs.FLAGS & Registers::CF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::ja(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JA requires 1 operand");
    if (!(regs.FLAGS & Registers::CF) && !(regs.FLAGS & Registers::ZF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jnp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNP requires 1 operand");
    if (!(regs.FLAGS & Registers::PF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jno(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNO requires 1 operand");
    if (!(regs.FLAGS & Registers::OF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jns(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNS requires 1 operand");
    if (!(regs.FLAGS & Registers::SF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::loop(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOP requires 1 operand");
    regs.CX.x--;
    if (regs.CX.x != 0)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::loopz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOPZ requires 1 operand");
    regs.CX.x--;
    if (regs.CX.x != 0 && (regs.FLAGS & Registers::ZF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::loopnz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOPNZ requires 1 operand");
    regs.CX.x--;
    if (regs.CX.x != 0 && !(regs.FLAGS & Registers::ZF))
        regs.IP = getValue(operands[0]);
}

void Emulator8086::jcxz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JCXZ requires 1 operand");
    if (regs.CX.x == 0)
        regs.IP = getValue(operands[0]);
}

void Emulator8086::int_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("INT requires 1 operand");
    throw std::runtime_error("Interrupt handling not implemented");
}

void Emulator8086::into(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("INTO takes no operands");
    if (regs.FLAGS & Registers::OF)
        throw std::runtime_error("Interrupt handling not implemented");
}

void Emulator8086::iret(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("IRET takes no operands");
    regs.IP = readMemoryWord(regs.SP);
    regs.SP += 2;
    regs.CS = readMemoryWord(regs.SP);
    regs.SP += 2;
    regs.FLAGS = readMemoryWord(regs.SP);
    regs.SP += 2;
}

void Emulator8086::rcl(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("RCL requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool oldCF = regs.FLAGS & Registers::CF;
            regs.FLAGS = (dest & 0x80) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest = (dest << 1) | oldCF;
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool oldCF = regs.FLAGS & Registers::CF;
            regs.FLAGS = (dest & 0x8000) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest = (dest << 1) | oldCF;
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::rcr(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("RCR requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool oldCF = regs.FLAGS & Registers::CF;
            regs.FLAGS = (dest & 0x01) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest = (dest >> 1) | (oldCF ? 0x80 : 0);
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool oldCF = regs.FLAGS & Registers::CF;
            regs.FLAGS = (dest & 0x0001) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest = (dest >> 1) | (oldCF ? 0x8000 : 0);
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::rol(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ROL requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool msb = dest & 0x80;
            dest = (dest << 1) | (msb ? 1 : 0);
            regs.FLAGS = msb ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool msb = dest & 0x8000;
            dest = (dest << 1) | (msb ? 1 : 0);
            regs.FLAGS = msb ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::ror(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ROR requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool lsb = dest & 0x01;
            dest = (dest >> 1) | (lsb ? 0x80 : 0);
            regs.FLAGS = lsb ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            bool lsb = dest & 0x0001;
            dest = (dest >> 1) | (lsb ? 0x8000 : 0);
            regs.FLAGS = lsb ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::sal(const std::vector<std::string> &operands) { shl(operands); }

void Emulator8086::sar(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SAR requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        int8_t &dest = reinterpret_cast<int8_t &>(getRegister8(operands[0]));
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x01) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest >>= 1;
        }
        updateFlags(static_cast<uint8_t>(dest), true, false);
    }
    else
    {
        int16_t &dest = reinterpret_cast<int16_t &>(getRegister(operands[0]));
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x0001) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest >>= 1;
        }
        updateFlags(static_cast<uint16_t>(dest), false, false);
    }
}

void Emulator8086::shl(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHL requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x80) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest <<= 1;
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x8000) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest <<= 1;
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::shr(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SHR requires 2 operands");
    uint8_t count = (operands[1] == "CL") ? regs.CX.l : std::stoi(operands[1]);
    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x01) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest >>= 1;
        }
        updateFlags(dest, true, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        for (uint8_t i = 0; i < count; i++)
        {
            regs.FLAGS = (dest & 0x0001) ? (regs.FLAGS | Registers::CF) : (regs.FLAGS & ~Registers::CF);
            dest >>= 1;
        }
        updateFlags(dest, false, false);
    }
}

void Emulator8086::clc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLC takes no operands");
    regs.FLAGS &= ~Registers::CF;
}

void Emulator8086::cmc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMC takes no operands");
    regs.FLAGS ^= Registers::CF;
}

void Emulator8086::stc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STC takes no operands");
    regs.FLAGS |= Registers::CF;
}

void Emulator8086::cld(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLD takes no operands");
    regs.FLAGS &= ~Registers::DF;
}

void Emulator8086::std(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STD takes no operands");
    regs.FLAGS |= Registers::DF;
}

void Emulator8086::cli(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLI takes no operands");
    regs.FLAGS &= ~Registers::IF;
}

void Emulator8086::sti(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STI takes no operands");
    regs.FLAGS |= Registers::IF;
}

void Emulator8086::hlt(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("HLT takes no operands");
    std::cout << "Emulator halted\n";
}

void Emulator8086::wait(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("WAIT takes no operands");
}

void Emulator8086::esc(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("ESC requires 1 operand");
    throw std::runtime_error("ESC not implemented");
}

void Emulator8086::lock(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LOCK takes no operands");
}

void Emulator8086::nop(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("NOP takes no operands");
}

void Emulator8086::in_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("IN requires 2 operands");
    throw std::runtime_error("I/O operations not implemented");
}

void Emulator8086::out(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OUT requires 2 operands");
    throw std::runtime_error("I/O operations not implemented");
}

void Emulator8086::rep(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("REP takes no operands");
    throw std::runtime_error("REP prefix not implemented in this text-based emulator");
}

void Emulator8086::repe(const std::vector<std::string> &operands) { rep(operands); }
void Emulator8086::repne(const std::vector<std::string> &operands) { rep(operands); }
void Emulator8086::repz(const std::vector<std::string> &operands) { rep(operands); }
void Emulator8086::repnz(const std::vector<std::string> &operands) { rep(operands); }

void Emulator8086::xlat(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("XLAT takes no operands");
    regs.AX.l = readMemoryByte(regs.BX.x + regs.AX.l);
}

void Emulator8086::xlatb(const std::vector<std::string> &operands) { xlat(operands); }

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
        if (!operand.empty())
            operands.push_back(operand);
    }
    auto it = instructions.find(opcode);
    if (it == instructions.end())
        throw std::runtime_error("Unknown instruction: " + opcode);
    it->second(operands);
}

void Emulator8086::displayRegisters()
{
    std::cout << std::hex << std::uppercase << std::setfill('0')
              << "AX=" << std::setw(4) << regs.AX.x << " (AH=" << std::setw(2) << static_cast<uint16_t>(regs.AX.h)
              << ", AL=" << std::setw(2) << static_cast<uint16_t>(regs.AX.l) << ")\n"
              << "BX=" << std::setw(4) << regs.BX.x << " (BH=" << std::setw(2) << static_cast<uint16_t>(regs.BX.h)
              << ", BL=" << std::setw(2) << static_cast<uint16_t>(regs.BX.l) << ")\n"
              << "CX=" << std::setw(4) << regs.CX.x << " (CH=" << std::setw(2) << static_cast<uint16_t>(regs.CX.h)
              << ", CL=" << std::setw(2) << static_cast<uint16_t>(regs.CX.l) << ")\n"
              << "DX=" << std::setw(4) << regs.DX.x << " (DH=" << std::setw(2) << static_cast<uint16_t>(regs.DX.h)
              << ", DL=" << std::setw(2) << static_cast<uint16_t>(regs.DX.l) << ")\n"
              << "SI=" << std::setw(4) << regs.SI << "  "
              << "DI=" << std::setw(4) << regs.DI << "  "
              << "BP=" << std::setw(4) << regs.BP << "  "
              << "SP=" << std::setw(4) << regs.SP << "\n"
              << "CS=" << std::setw(4) << regs.CS << "  "
              << "DS=" << std::setw(4) << regs.DS << "  "
              << "ES=" << std::setw(4) << regs.ES << "  "
              << "SS=" << std::setw(4) << regs.SS << "\n"
              << "IP=" << std::setw(4) << regs.IP << "  "
              << "FLAGS=" << std::setw(4) << regs.FLAGS
              << " [" << (regs.FLAGS & Registers::OF ? "O" : "-")
              << (regs.FLAGS & Registers::DF ? "D" : "-")
              << (regs.FLAGS & Registers::IF ? "I" : "-")
              << (regs.FLAGS & Registers::TF ? "T" : "-")
              << (regs.FLAGS & Registers::SF ? "S" : "-")
              << (regs.FLAGS & Registers::ZF ? "Z" : "-")
              << (regs.FLAGS & Registers::AF ? "A" : "-")
              << (regs.FLAGS & Registers::PF ? "P" : "-")
              << (regs.FLAGS & Registers::CF ? "C" : "-") << "]\n";
}

void Emulator8086::displayStack()
{
    std::cout << "\nStack contents:\n";
    if (regs.SP == 0xFFFE)
    {
        std::cout << "Stack is empty\n";
        return;
    }
    for (uint16_t i = regs.SP; i < 0xFFFE; i += 2)
    {
        std::cout << std::hex << std::uppercase << std::setfill('0')
                  << "SP+" << std::setw(4) << (i - regs.SP) << ": "
                  << std::setw(4) << readMemoryWord(i) << '\n';
    }
}

void Emulator8086::displayMemory(uint16_t address, uint16_t count)
{
    std::cout << "\nMemory dump from " << std::hex << std::uppercase << std::setfill('0')
              << std::setw(4) << address << ":\n";
    for (uint16_t i = 0; i < count; i++)
    {
        if (i % 16 == 0)
            std::cout << std::setw(4) << (address + i) << ": ";
        std::cout << std::setw(2) << static_cast<int>(readMemoryByte(address + i)) << ' ';
        if (i % 16 == 15 || i == count - 1)
            std::cout << '\n';
    }
}

void Emulator8086::displayHelp()
{
    std::cout << "8086 Emulator - Supported Instructions:\n";
    std::cout << "----------------------------------------\n";

    std::cout << "Data Transfer Instructions:\n";
    std::cout << "  MOV dest,src       - Move data from src to dest (reg/mem/imm)\n";
    std::cout << "  PUSH src           - Push 16-bit value onto stack\n";
    std::cout << "  POP dest           - Pop 16-bit value from stack to dest\n";
    std::cout << "  XCHG op1,op2       - Exchange contents of two operands\n";
    std::cout << "  LEA reg,[mem]      - Load effective address into register\n";
    std::cout << "  LDS reg,[mem]      - Load DS and register from memory\n";
    std::cout << "  LES reg,[mem]      - Load ES and register from memory\n";
    std::cout << "  LAHF               - Load AH from flags\n";
    std::cout << "  SAHF               - Store AH into flags\n";
    std::cout << "  PUSHF              - Push flags onto stack\n";
    std::cout << "  POPF               - Pop flags from stack\n";
    std::cout << "  PUSHA              - Push all general-purpose registers\n";
    std::cout << "  POPA               - Pop all general-purpose registers\n";
    std::cout << "  IN dest,port       - Input from port (not implemented)\n";
    std::cout << "  OUT port,src       - Output to port (not implemented)\n";
    std::cout << "  XLAT               - Translate byte using table (BX+AL)\n";
    std::cout << "  XLATB              - Same as XLAT\n";

    std::cout << "\nArithmetic Instructions:\n";
    std::cout << "  ADD dest,src       - Add src to dest\n";
    std::cout << "  ADC dest,src       - Add with carry\n";
    std::cout << "  INC op             - Increment operand by 1\n";
    std::cout << "  AAA                - ASCII adjust after addition\n";
    std::cout << "  DAA                - Decimal adjust after addition\n";
    std::cout << "  SUB dest,src       - Subtract src from dest\n";
    std::cout << "  SBB dest,src       - Subtract with borrow\n";
    std::cout << "  DEC op             - Decrement operand by 1\n";
    std::cout << "  NEG op             - Negate operand (two's complement)\n";
    std::cout << "  AAS                - ASCII adjust after subtraction\n";
    std::cout << "  DAS                - Decimal adjust after subtraction\n";
    std::cout << "  MUL op             - Unsigned multiply (AX or DX:AX)\n";
    std::cout << "  IMUL op            - Signed multiply (AX or DX:AX)\n";
    std::cout << "  AAM                - ASCII adjust after multiplication\n";
    std::cout << "  DIV op             - Unsigned divide (AX or DX:AX)\n";
    std::cout << "  IDIV op            - Signed divide (AX or DX:AX)\n";
    std::cout << "  AAD                - ASCII adjust before division\n";
    std::cout << "  CBW                - Convert byte to word (sign-extend AL to AX)\n";
    std::cout << "  CWD                - Convert word to doubleword (sign-extend AX to DX:AX)\n";

    std::cout << "\nLogical Instructions:\n";
    std::cout << "  AND dest,src       - Bitwise AND\n";
    std::cout << "  OR dest,src        - Bitwise OR\n";
    std::cout << "  XOR dest,src       - Bitwise XOR\n";
    std::cout << "  NOT op             - Bitwise NOT\n";
    std::cout << "  TEST dest,src      - Test bits (AND without storing result)\n";
    std::cout << "  CMP dest,src       - Compare (subtract without storing result)\n";

    std::cout << "\nShift and Rotate Instructions:\n";
    std::cout << "  RCL op,count       - Rotate left through carry\n";
    std::cout << "  RCR op,count       - Rotate right through carry\n";
    std::cout << "  ROL op,count       - Rotate left\n";
    std::cout << "  ROR op,count       - Rotate right\n";
    std::cout << "  SAL op,count       - Shift arithmetic left (same as SHL)\n";
    std::cout << "  SAR op,count       - Shift arithmetic right\n";
    std::cout << "  SHL op,count       - Shift logical left\n";
    std::cout << "  SHR op,count       - Shift logical right\n";

    std::cout << "\nString Instructions:\n";
    std::cout << "  MOVSB              - Move byte from SI to DI\n";
    std::cout << "  MOVSW              - Move word from SI to DI\n";
    std::cout << "  CMPSB              - Compare byte at SI and DI\n";
    std::cout << "  CMPSW              - Compare word at SI and DI\n";
    std::cout << "  SCASB              - Scan byte (compare AL with DI)\n";
    std::cout << "  SCASW              - Scan word (compare AX with DI)\n";
    std::cout << "  LODSB              - Load byte from SI to AL\n";
    std::cout << "  LODSW              - Load word from SI to AX\n";
    std::cout << "  STOSB              - Store AL to DI\n";
    std::cout << "  STOSW              - Store AX to DI\n";

    std::cout << "\nControl Transfer Instructions:\n";
    std::cout << "  CALL addr          - Call subroutine\n";
    std::cout << "  JMP addr           - Unconditional jump\n";
    std::cout << "  RET                - Return from subroutine\n";
    std::cout << "  RETF               - Return far (pops CS and IP)\n";
    std::cout << "  JE addr            - Jump if equal (ZF=1)\n";
    std::cout << "  JL addr            - Jump if less (SF!=OF)\n";
    std::cout << "  JLE addr           - Jump if less or equal (ZF=1 or SF!=OF)\n";
    std::cout << "  JB addr            - Jump if below (CF=1)\n";
    std::cout << "  JBE addr           - Jump if below or equal (CF=1 or ZF=1)\n";
    std::cout << "  JP addr            - Jump if parity (PF=1)\n";
    std::cout << "  JO addr            - Jump if overflow (OF=1)\n";
    std::cout << "  JS addr            - Jump if sign (SF=1)\n";
    std::cout << "  JNE addr           - Jump if not equal (ZF=0)\n";
    std::cout << "  JNL addr           - Jump if not less (SF=OF)\n";
    std::cout << "  JG addr            - Jump if greater (ZF=0 and SF=OF)\n";
    std::cout << "  JNB addr           - Jump if not below (CF=0)\n";
    std::cout << "  JA addr            - Jump if above (CF=0 and ZF=0)\n";
    std::cout << "  JNP addr           - Jump if not parity (PF=0)\n";
    std::cout << "  JNO addr           - Jump if no overflow (OF=0)\n";
    std::cout << "  JNS addr           - Jump if not sign (SF=0)\n";
    std::cout << "  LOOP addr          - Loop while CX != 0\n";
    std::cout << "  LOOPZ addr         - Loop while CX != 0 and ZF=1\n";
    std::cout << "  LOOPNZ addr        - Loop while CX != 0 and ZF=0\n";
    std::cout << "  JCXZ addr          - Jump if CX = 0\n";

    std::cout << "\nInterrupt Instructions:\n";
    std::cout << "  INT num            - Software interrupt (not implemented)\n";
    std::cout << "  INTO               - Interrupt on overflow (not implemented)\n";
    std::cout << "  IRET               - Return from interrupt\n";

    std::cout << "\nFlag Control Instructions:\n";
    std::cout << "  CLC                - Clear carry flag\n";
    std::cout << "  CMC                - Complement carry flag\n";
    std::cout << "  STC                - Set carry flag\n";
    std::cout << "  CLD                - Clear direction flag\n";
    std::cout << "  STD                - Set direction flag\n";
    std::cout << "  CLI                - Clear interrupt flag\n";
    std::cout << "  STI                - Set interrupt flag\n";

    std::cout << "\nMiscellaneous Instructions:\n";
    std::cout << "  HLT                - Halt execution\n";
    std::cout << "  WAIT               - Wait for external signal (no-op)\n";
    std::cout << "  ESC op             - Escape to coprocessor (not implemented)\n";
    std::cout << "  LOCK               - Lock bus (no-op)\n";
    std::cout << "  NOP                - No operation\n";
    std::cout << "  REP                - Repeat string operation (not implemented)\n";
    std::cout << "  REPE/REPZ          - Repeat while equal (not implemented)\n";
    std::cout << "  REPNE/REPNZ        - Repeat while not equal (not implemented)\n";

    std::cout << "\nEmulator Commands:\n";
    std::cout << "  reg                - Display registers\n";
    std::cout << "  stack              - Display stack contents\n";
    std::cout << "  mem addr count     - Display memory (e.g., 'mem 100h 20h')\n";
    std::cout << "  ?                  - Show this help\n";
    std::cout << "  :3xit              - Exit emulator\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Notes: Use 'h' suffix for hex (e.g., 10h), memory as [BX+SI+offset]\n";
}