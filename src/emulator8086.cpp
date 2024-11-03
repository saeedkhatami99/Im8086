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
    instructions["PUSH"] = std::bind(&Emulator8086::push, this, std::placeholders::_1);
    instructions["POP"] = std::bind(&Emulator8086::pop, this, std::placeholders::_1);
    instructions["JMP"] = std::bind(&Emulator8086::jmp, this, std::placeholders::_1);
    instructions["CMP"] = std::bind(&Emulator8086::cmp, this, std::placeholders::_1);
    instructions["JE"] = std::bind(&Emulator8086::je, this, std::placeholders::_1);
    instructions["JNE"] = std::bind(&Emulator8086::jne, this, std::placeholders::_1);
    instructions["JG"] = std::bind(&Emulator8086::jg, this, std::placeholders::_1);
    instructions["JL"] = std::bind(&Emulator8086::jl, this, std::placeholders::_1);
    instructions["INC"] = std::bind(&Emulator8086::inc, this, std::placeholders::_1);
    instructions["DEC"] = std::bind(&Emulator8086::dec, this, std::placeholders::_1);
    instructions["AND"] = std::bind(&Emulator8086::and_op, this, std::placeholders::_1);
    instructions["OR"] = std::bind(&Emulator8086::or_op, this, std::placeholders::_1);
    instructions["XOR"] = std::bind(&Emulator8086::xor_op, this, std::placeholders::_1);
    instructions["NOT"] = std::bind(&Emulator8086::not_op, this, std::placeholders::_1);
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
    if (operand.back() == 'h')
    {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16);
    }
    else
    {
        if (operand[0] >= '0' && operand[0] <= '9')
        {
            return std::stoi(operand);
        }
        return getRegister(operand);
    }
}

uint8_t Emulator8086::getValue8(const std::string &operand)
{
    if (operand.back() == 'h')
    {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16) & 0xFF;
    }
    else if (operand[0] >= '0' && operand[0] <= '9')
    {
        return std::stoi(operand) & 0xFF;
    }
    return getRegister8(operand);
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

    if (is8BitRegister(dest))
    {
        if (is8BitRegister(source))
        {
            getRegister8(dest) = getRegister8(source);
        }
        else if (!isMemoryOperand(source))
        {
            getRegister8(dest) = static_cast<uint8_t>(getValue(source) & 0xFF);
        }
        else
        {
            MemoryOperand memOp = parseMemoryOperand(source);
            uint16_t address = calculateEffectiveAddress(memOp);
            getRegister8(dest) = readMemoryByte(address);
        }
        return;
    }

    if (!isMemoryOperand(dest) && !isMemoryOperand(source))
    {
        getRegister(dest) = getValue(source);
    }
    else if (!isMemoryOperand(dest) && isMemoryOperand(source))
    {
        MemoryOperand memOp = parseMemoryOperand(source);
        uint16_t address = calculateEffectiveAddress(memOp);
        getRegister(dest) = readMemoryWord(address);
    }
    else if (isMemoryOperand(dest) && !isMemoryOperand(source))
    {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);
        writeMemoryWord(address, getValue(source));
    }
    else
    {
        throw std::runtime_error("Memory to memory MOV is not allowed");
    }
}

void Emulator8086::add(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("ADD requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t value = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = dest + value;
        dest = result & 0xFF;
        updateFlags(result, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest + getValue(operands[1]);
        dest = result & 0xFFFF;
        updateFlags(result, true);
    }
}

void Emulator8086::sub(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("SUB requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t value = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = dest - value;
        dest = result & 0xFF;
        updateFlags(result, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest - getValue(operands[1]);
        dest = result & 0xFFFF;
        updateFlags(result, true);
    }
}

void Emulator8086::push(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("PUSH requires 1 operand");

    if (is8BitRegister(operands[0]))
    {
        throw std::runtime_error("PUSH cannot operate on 8-bit registers");
    }
    else
    {
        uint16_t value = getValue(operands[0]);
        regs.SP -= 2;
        writeMemoryWord(regs.SP, value);
    }
}

void Emulator8086::pop(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("POP requires 1 operand");

    if (is8BitRegister(operands[0]))
    {
        throw std::runtime_error("POP cannot operate on 8-bit registers");
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        dest = readMemoryWord(regs.SP);
        regs.SP += 2;
    }
}

void Emulator8086::cmp(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("CMP requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t value1 = getRegister8(operands[0]);
        uint8_t value2 = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = value1 - value2;
        updateFlags(result, true);
    }
    else
    {
        uint16_t value1 = getRegister(operands[0]);
        uint16_t value2 = getValue(operands[1]);
        uint32_t result = value1 - value2;
        updateFlags(result, true);
    }
}

void Emulator8086::jmp(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JMP requires 1 operand");

    regs.IP = getValue(operands[0]);
}

void Emulator8086::je(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JE requires 1 operand");

    if (regs.FLAGS & Registers::ZF)
    {
        regs.IP = getValue(operands[0]);
    }
}

void Emulator8086::jne(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNE requires 1 operand");

    if (!(regs.FLAGS & Registers::ZF))
    {
        regs.IP = getValue(operands[0]);
    }
}

void Emulator8086::jg(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JG requires 1 operand");

    if (!(regs.FLAGS & Registers::ZF) && !(regs.FLAGS & Registers::SF))
    {
        regs.IP = getValue(operands[0]);
    }
}

void Emulator8086::jl(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JL requires 1 operand");

    if (regs.FLAGS & Registers::SF)
    {
        regs.IP = getValue(operands[0]);
    }
}

void Emulator8086::inc(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("INC requires 1 operand");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint16_t result = dest + 1;
        dest = result & 0xFF;
        updateFlags(result, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest + 1;
        dest = result & 0xFFFF;
        updateFlags(result, true);
    }
}

void Emulator8086::dec(std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("DEC requires 1 operand");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint16_t result = dest - 1;
        dest = result & 0xFF;
        updateFlags(result, true);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest - 1;
        dest = result & 0xFFFF;
        updateFlags(result, true);
    }
}

void Emulator8086::and_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("AND requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t value = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = dest & value;
        dest = result & 0xFF;
        updateFlags(result, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest & getValue(operands[1]);
        dest = result & 0xFFFF;
        updateFlags(result, false);
    }
}

void Emulator8086::or_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OR requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t value = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = dest | value;
        dest = result & 0xFF;
        updateFlags(result, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest | getValue(operands[1]);
        dest = result & 0xFFFF;
        updateFlags(result, false);
    }
}

void Emulator8086::xor_op(std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("XOR requires 2 operands");

    if (is8BitRegister(operands[0]))
    {
        uint8_t &dest = getRegister8(operands[0]);
        uint8_t value = is8BitRegister(operands[1]) ? getRegister8(operands[1]) : (getValue(operands[1]) & 0xFF);
        uint16_t result = dest ^ value;
        dest = result & 0xFF;
        updateFlags(result, false);
    }
    else
    {
        uint16_t &dest = getRegister(operands[0]);
        uint32_t result = dest ^ getValue(operands[1]);
        dest = result & 0xFFFF;
        updateFlags(result, false);
    }
}

void Emulator8086::not_op(std::vector<std::string> &operands)
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

// std::string toBinaryString(uint16_t value) {
//     return std::bitset<16>(value).to_string();
// }

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
              << (regs.FLAGS & Registers::CF ? "C" : "-")
              << "]\n";
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
    std::cout << "16. NOT <operand>               - Performs bitwise NOT on the operand\n";
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