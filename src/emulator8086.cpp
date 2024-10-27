#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include "emulator8086.h"

Emulator8086::Emulator8086(size_t memSize) : memory(memSize, 0) {
    initializeInstructions();
}

void Emulator8086::initializeInstructions() {
    instructions["MOV"] = &Emulator8086::mov;
    instructions["ADD"] = &Emulator8086::add;
    instructions["SUB"] = &Emulator8086::sub;
    instructions["PUSH"] = &Emulator8086::push;
    instructions["POP"] = &Emulator8086::pop;
    instructions["JMP"] = &Emulator8086::jmp;
    instructions["CMP"] = &Emulator8086::cmp;
    instructions["JE"] = &Emulator8086::je;
    instructions["JNE"] = &Emulator8086::jne;
    instructions["JG"] = &Emulator8086::jg;
    instructions["JL"] = &Emulator8086::jl;
    instructions["INC"] = &Emulator8086::inc;
    instructions["DEC"] = &Emulator8086::dec;
    instructions["AND"] = &Emulator8086::and_op;
    instructions["OR"] = &Emulator8086::or_op;
    instructions["XOR"] = &Emulator8086::xor_op;
    instructions["NOT"] = &Emulator8086::not_op;
}

uint16_t& Emulator8086::getRegister(const std::string& reg) {
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);
    
    if (upperReg == "AX") return regs.AX;
    if (upperReg == "BX") return regs.BX;
    if (upperReg == "CX") return regs.CX;
    if (upperReg == "DX") return regs.DX;
    if (upperReg == "SI") return regs.SI;
    if (upperReg == "DI") return regs.DI;
    if (upperReg == "BP") return regs.BP;
    if (upperReg == "SP") return regs.SP;
    throw std::runtime_error("Invalid register: " + reg);
}

MemoryOperand Emulator8086::parseMemoryOperand(const std::string& operand) {
    MemoryOperand result;
    std::string inner = operand.substr(1, operand.length() - 2); // Remove []
    
    std::vector<std::string> parts;
    std::string current;
    bool negative = false;
    
    for (size_t i = 0; i < inner.length(); i++) {
        if (inner[i] == '+' || inner[i] == '-') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            if (inner[i] == '-') negative = true;
        } else {
            current += inner[i];
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }

    for (const auto& part : parts) {
        std::string upperPart = part;
        std::transform(upperPart.begin(), upperPart.end(), upperPart.begin(), ::toupper);
        
        if (upperPart == "BX") {
            result.hasBase = true;
            result.base = regs.BX;
        }
        else if (upperPart == "BP") {
            result.hasBase = true;
            result.base = regs.BP;
        }
        else if (upperPart == "SI") {
            result.hasIndex = true;
            result.index = regs.SI;
        }
        else if (upperPart == "DI") {
            result.hasIndex = true;
            result.index = regs.DI;
        }
        else {
            result.hasDisplacement = true;
            result.displacement = std::stoi(part, nullptr, 16);
            if (negative) result.displacement = -result.displacement;
        }
    }
    
    return result;
}

uint16_t Emulator8086::calculateEffectiveAddress(const MemoryOperand& memOp) {
    uint16_t ea = 0;
    
    if (memOp.hasBase) {
        ea += memOp.base;
    }
    
    if (memOp.hasIndex) {
        ea += memOp.index;
    }
    
    if (memOp.hasDisplacement) {
        ea += memOp.displacement;
    }
    
    return ea;
}

uint16_t Emulator8086::readMemoryWord(uint16_t address) {
    return (memory[address + 1] << 8) | memory[address];
}

void Emulator8086::writeMemoryWord(uint16_t address, uint16_t value) {
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

uint8_t Emulator8086::readMemoryByte(uint16_t address) {
    return memory[address];
}

void Emulator8086::writeMemoryByte(uint16_t address, uint8_t value) {
    memory[address] = value;
}

bool Emulator8086::isMemoryOperand(const std::string& operand) {
    return operand[0] == '[' && operand.back() == ']';
}

uint16_t Emulator8086::getValue(const std::string& operand) {
    if (operand[0] >= '0' && operand[0] <= '9') {
        return std::stoi(operand, nullptr, 16);
    }
    return getRegister(operand);
}

void Emulator8086::updateFlags(uint16_t result, bool checkCarry) {
    if (result == 0) {
        regs.FLAGS |= Registers::ZF;
    } else {
        regs.FLAGS &= ~Registers::ZF;
    }

    if (result & 0x8000) {
        regs.FLAGS |= Registers::SF;
    } else {
        regs.FLAGS &= ~Registers::SF;
    }

    if (checkCarry && (result > 0xFFFF)) {
        regs.FLAGS |= Registers::CF;
    } else if (checkCarry) {
        regs.FLAGS &= ~Registers::CF;
    }
}

void Emulator8086::mov(std::vector<std::string>& operands) {
    if (operands.size() != 2) {
        throw std::runtime_error("MOV requires 2 operands");
    }

    const std::string& dest = operands[0];
    const std::string& source = operands[1];

    if (!isMemoryOperand(dest) && !isMemoryOperand(source)) {
        getRegister(dest) = getValue(source);
    }
    else if (!isMemoryOperand(dest) && isMemoryOperand(source)) {
        MemoryOperand memOp = parseMemoryOperand(source);
        uint16_t address = calculateEffectiveAddress(memOp);
        getRegister(dest) = readMemoryWord(address);
    }
    else if (isMemoryOperand(dest) && !isMemoryOperand(source)) {
        MemoryOperand memOp = parseMemoryOperand(dest);
        uint16_t address = calculateEffectiveAddress(memOp);
        writeMemoryWord(address, getValue(source));
    }
    else {
        throw std::runtime_error("Memory to memory MOV is not allowed");
    }
}

void Emulator8086::add(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("ADD requires 2 operands");
    uint16_t& dest = getRegister(operands[0]);
    uint32_t result = dest + getValue(operands[1]);
    dest = result & 0xFFFF;
    updateFlags(result, true);
}

void Emulator8086::sub(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("SUB requires 2 operands");
    uint16_t& dest = getRegister(operands[0]);
    uint16_t value = getValue(operands[1]);
    uint32_t result = dest - value;
    dest = result & 0xFFFF;
    updateFlags(result, true);
}

void Emulator8086::push(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("PUSH requires 1 operand");
    uint16_t value = getValue(operands[0]);
    regs.SP -= 2;
    writeMemoryWord(regs.SP, value);
}

void Emulator8086::pop(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("POP requires 1 operand");
    uint16_t value = readMemoryWord(regs.SP);
    getRegister(operands[0]) = value;
    regs.SP += 2;
}

void Emulator8086::cmp(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("CMP requires 2 operands");
    uint16_t value1 = getValue(operands[0]);
    uint16_t value2 = getValue(operands[1]);
    uint32_t result = value1 - value2;
    updateFlags(result, true);
}

void Emulator8086::jmp(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("JMP requires 1 operand");
    if (labels.find(operands[0]) == labels.end()) {
        throw std::runtime_error("Label not found: " + operands[0]);
    }
    regs.IP = labels[operands[0]];
}

void Emulator8086::je(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("JE requires 1 operand");
    if (regs.FLAGS & Registers::ZF) {
        jmp(operands);
    }
}

void Emulator8086::jne(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("JNE requires 1 operand");
    if (!(regs.FLAGS & Registers::ZF)) {
        jmp(operands);
    }
}

void Emulator8086::jg(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("JG requires 1 operand");
    if (!(regs.FLAGS & Registers::ZF) && 
        ((regs.FLAGS & Registers::SF) == (regs.FLAGS & Registers::OF))) {
        jmp(operands);
    }
}

void Emulator8086::jl(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("JL requires 1 operand");
    if ((regs.FLAGS & Registers::SF) != (regs.FLAGS & Registers::OF)) {
        jmp(operands);
    }
}

void Emulator8086::inc(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("INC requires 1 operand");
    uint16_t& dest = getRegister(operands[0]);
    uint32_t result = dest + 1;
    dest = result & 0xFFFF;
    updateFlags(result, true);
}

void Emulator8086::dec(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("DEC requires 1 operand");
    uint16_t& dest = getRegister(operands[0]);
    uint32_t result = dest - 1;
    dest = result & 0xFFFF;
    updateFlags(result, true);
}

void Emulator8086::and_op(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("AND requires 2 operands");
    uint16_t& dest = getRegister(operands[0]);
    dest &= getValue(operands[1]);
    updateFlags(dest, false);
}

void Emulator8086::or_op(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("OR requires 2 operands");
    uint16_t& dest = getRegister(operands[0]);
    dest |= getValue(operands[1]);
    updateFlags(dest, false);
}

void Emulator8086::xor_op(std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("XOR requires 2 operands");
    uint16_t& dest = getRegister(operands[0]);
    dest ^= getValue(operands[1]);
    updateFlags(dest, false);
}

void Emulator8086::not_op(std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("NOT requires 1 operand");
    uint16_t& dest = getRegister(operands[0]);
    dest = ~dest;
}

void Emulator8086::executeInstruction(const std::string& instruction) {
    std::istringstream iss(instruction);
    std::string opcode;
    iss >> opcode;
    
    std::transform(opcode.begin(), opcode.end(), opcode.begin(), ::toupper);
    
    std::vector<std::string> operands;
    std::string operand;
    while (std::getline(iss >> std::ws, operand, ',')) {
        // Remove leading/trailing whitespace
        operand.erase(0, operand.find_first_not_of(" \t"));
        operand.erase(operand.find_last_not_of(" \t") + 1);
        operands.push_back(operand);
    }
    
    auto it = instructions.find(opcode);
    if (it == instructions.end()) {
        throw std::runtime_error("Unknown instruction: " + opcode);
    }
    
    (this->*(it->second))(operands);
}

void Emulator8086::displayRegisters() {
    std::cout << std::hex << std::uppercase << std::setfill('0')
              << "AX=" << std::setw(4) << regs.AX << " "
              << "BX=" << std::setw(4) << regs.BX << " "
              << "CX=" << std::setw(4) << regs.CX << " "
              << "DX=" << std::setw(4) << regs.DX << "\n"
              << "SI=" << std::setw(4) << regs.SI << " "
              << "DI=" << std::setw(4) << regs.DI << " "
              << "BP=" << std::setw(4) << regs.BP << " "
              << "SP=" << std::setw(4) << regs.SP << "\n"
              << "FLAGS=" << std::setw(4) << regs.FLAGS << " "
              << "IP=" << std::setw(4) << regs.IP << "\n";
}

void Emulator8086::displayHelp() {
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

void Emulator8086::displayMemory(uint16_t start, uint16_t count) {
    for (uint16_t i = 0; i < count; i += 16) {
        std::cout << std::hex << std::uppercase << std::setfill('0')
                  << std::setw(4) << start + i << ": ";
        
        for (uint16_t j = 0; j < 16 && (i + j) < count; j++) {
            std::cout << std::setw(2) << static_cast<int>(memory[start + i + j]) << " ";
        }
        std::cout << "\n";
    }
}

void Emulator8086::displayStack(int count) {
    std::cout << "Stack:\n";
    for (int i = 0; i < count && regs.SP + i < 0xFFFE; i += 2) {
        uint16_t value = readMemoryWord(regs.SP + i);
        std::cout << std::hex << std::uppercase << std::setfill('0')
                  << "SP+" << std::setw(4) << i << ": "
                  << std::setw(4) << value << "\n";
    }
}