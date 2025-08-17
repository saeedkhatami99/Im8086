#include "instructions/arithmetic.h"

#include <stdexcept>

#include "emulator8086.h"

ArithmeticInstructions::ArithmeticInstructions(Emulator8086* emu) : emulator(emu) {}

void ArithmeticInstructions::add(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("ADD requires 2 operands");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint16_t result = dest + src;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, true);
    } else if (emulator->isMemoryOperand(operands[0])) {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1])) {
            uint8_t destVal = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            uint16_t result = destVal + src;
            emulator->writeMemoryByte(address, result & 0xFF);
            emulator->updateFlags(result, true, true);
        } else {
            uint16_t destVal = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            uint32_t result = destVal + src;
            emulator->writeMemoryWord(address, result & 0xFFFF);
            emulator->updateFlags(result, false, true);
        }
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint32_t result = dest + src;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::adc(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("ADC requires 2 operands");
    bool cf = emulator->getRegisters().FLAGS & Registers::CF;
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint16_t result = dest + src + cf;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, true);
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint32_t result = dest + src + cf;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::inc(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("INC requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint16_t result = dest + 1;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, false);
    } else if (emulator->isMemoryOperand(operands[0])) {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t destVal = emulator->readMemoryWord(address);
        uint32_t result = destVal + 1;
        emulator->writeMemoryWord(address, result & 0xFFFF);
        emulator->updateFlags(result, false, false);
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint32_t result = dest + 1;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, false);
    }
}

void ArithmeticInstructions::aaa(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("AAA takes no operands");
    if ((emulator->getRegisters().AX.bytes.l & 0x0F) > 9 ||
        (emulator->getRegisters().FLAGS & Registers::AF)) {
        emulator->getRegisters().AX.bytes.l += 6;
        emulator->getRegisters().AX.bytes.h += 1;
        emulator->getRegisters().FLAGS |= (Registers::AF | Registers::CF);
    } else {
        emulator->getRegisters().FLAGS &= ~(Registers::AF | Registers::CF);
    }
    emulator->getRegisters().AX.bytes.l &= 0x0F;
}

void ArithmeticInstructions::daa(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("DAA takes no operands");
    uint8_t oldAL = emulator->getRegisters().AX.bytes.l;
    bool oldCF = emulator->getRegisters().FLAGS & Registers::CF;
    emulator->getRegisters().FLAGS &= ~Registers::CF;
    if ((emulator->getRegisters().AX.bytes.l & 0x0F) > 9 ||
        (emulator->getRegisters().FLAGS & Registers::AF)) {
        emulator->getRegisters().AX.bytes.l += 6;
        emulator->getRegisters().FLAGS |= Registers::AF;
    }
    if ((oldAL > 0x99) || oldCF) {
        emulator->getRegisters().AX.bytes.l += 0x60;
        emulator->getRegisters().FLAGS |= Registers::CF;
    }
}

void ArithmeticInstructions::sub(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("SUB requires 2 operands");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint16_t result = dest - src;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, true);
    } else if (emulator->isMemoryOperand(operands[0])) {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        if (emulator->is8BitRegister(operands[1])) {
            uint8_t destVal = emulator->readMemoryByte(address);
            uint8_t src = emulator->getRegister8(operands[1]);
            uint16_t result = destVal - src;
            emulator->writeMemoryByte(address, result & 0xFF);
            emulator->updateFlags(result, true, true);
        } else {
            uint16_t destVal = emulator->readMemoryWord(address);
            uint16_t src = emulator->getValue(operands[1]);
            uint32_t result = destVal - src;
            emulator->writeMemoryWord(address, result & 0xFFFF);
            emulator->updateFlags(result, false, true);
        }
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint32_t result = dest - src;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::sbb(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("SBB requires 2 operands");
    bool cf = emulator->getRegisters().FLAGS & Registers::CF;
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint8_t src = emulator->getValue8(operands[1]);
        uint16_t result = dest - src - cf;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, true);
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint16_t src = emulator->getValue(operands[1]);
        uint32_t result = dest - src - cf;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::dec(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("DEC requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint16_t result = dest - 1;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, false);
    } else if (emulator->isMemoryOperand(operands[0])) {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t destVal = emulator->readMemoryWord(address);
        uint32_t result = destVal - 1;
        emulator->writeMemoryWord(address, result & 0xFFFF);
        emulator->updateFlags(result, false, false);
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint32_t result = dest - 1;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, false);
    }
}

void ArithmeticInstructions::neg(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("NEG requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t& dest = emulator->getRegister8(operands[0]);
        uint16_t result = 0 - dest;
        dest = result & 0xFF;
        emulator->updateFlags(result, true, true);
    } else if (emulator->isMemoryOperand(operands[0])) {
        MemoryOperand memOp = emulator->parseMemoryOperand(operands[0]);
        uint16_t address = emulator->calculateEffectiveAddress(memOp);
        uint16_t destVal = emulator->readMemoryWord(address);
        uint32_t result = 0 - destVal;
        emulator->writeMemoryWord(address, result & 0xFFFF);
        emulator->updateFlags(result, false, true);
    } else {
        uint16_t& dest = emulator->getRegister(operands[0]);
        uint32_t result = 0 - dest;
        dest = result & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::aas(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("AAS takes no operands");
    if ((emulator->getRegisters().AX.bytes.l & 0x0F) > 9 ||
        (emulator->getRegisters().FLAGS & Registers::AF)) {
        emulator->getRegisters().AX.bytes.l -= 6;
        emulator->getRegisters().AX.bytes.h -= 1;
        emulator->getRegisters().FLAGS |= (Registers::AF | Registers::CF);
    } else {
        emulator->getRegisters().FLAGS &= ~(Registers::AF | Registers::CF);
    }
    emulator->getRegisters().AX.bytes.l &= 0x0F;
}

void ArithmeticInstructions::das(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("DAS takes no operands");
    uint8_t oldAL = emulator->getRegisters().AX.bytes.l;
    bool oldCF = emulator->getRegisters().FLAGS & Registers::CF;
    emulator->getRegisters().FLAGS &= ~Registers::CF;
    if ((emulator->getRegisters().AX.bytes.l & 0x0F) > 9 ||
        (emulator->getRegisters().FLAGS & Registers::AF)) {
        emulator->getRegisters().AX.bytes.l -= 6;
        emulator->getRegisters().FLAGS |= Registers::AF;
    }
    if ((oldAL > 0x99) || oldCF) {
        emulator->getRegisters().AX.bytes.l -= 0x60;
        emulator->getRegisters().FLAGS |= Registers::CF;
    }
}

void ArithmeticInstructions::mul(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("MUL requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        uint16_t result = emulator->getRegisters().AX.bytes.l * emulator->getRegister8(operands[0]);
        emulator->getRegisters().AX.x = result;
        emulator->updateFlags(result, false, true);
    } else {
        uint32_t result = emulator->getRegisters().AX.x * emulator->getValue(operands[0]);
        emulator->getRegisters().AX.x = result & 0xFFFF;
        emulator->getRegisters().DX.x = (result >> 16) & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::imul(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("IMUL requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        int16_t result = static_cast<int8_t>(emulator->getRegisters().AX.bytes.l) *
                         static_cast<int8_t>(emulator->getRegister8(operands[0]));
        emulator->getRegisters().AX.x = result;
        emulator->updateFlags(result, false, true);
    } else {
        int32_t result = static_cast<int16_t>(emulator->getRegisters().AX.x) *
                         static_cast<int16_t>(emulator->getValue(operands[0]));
        emulator->getRegisters().AX.x = result & 0xFFFF;
        emulator->getRegisters().DX.x = (result >> 16) & 0xFFFF;
        emulator->updateFlags(result, false, true);
    }
}

void ArithmeticInstructions::aam(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("AAM takes no operands");
    uint8_t al = emulator->getRegisters().AX.bytes.l;
    emulator->getRegisters().AX.bytes.h = al / 10;
    emulator->getRegisters().AX.bytes.l = al % 10;
    emulator->updateFlags(emulator->getRegisters().AX.x, false, false);
}

void ArithmeticInstructions::div(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("DIV requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        uint8_t divisor = emulator->getRegister8(operands[0]);
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        uint16_t dividend = emulator->getRegisters().AX.x;
        emulator->getRegisters().AX.bytes.l = dividend / divisor;
        emulator->getRegisters().AX.bytes.h = dividend % divisor;
    } else {
        uint16_t divisor = emulator->getValue(operands[0]);
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        uint32_t dividend = (emulator->getRegisters().DX.x << 16) | emulator->getRegisters().AX.x;
        emulator->getRegisters().AX.x = dividend / divisor;
        emulator->getRegisters().DX.x = dividend % divisor;
    }
}

void ArithmeticInstructions::idiv(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("IDIV requires 1 operand");
    if (emulator->is8BitRegister(operands[0])) {
        int8_t divisor = static_cast<int8_t>(emulator->getRegister8(operands[0]));
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        int16_t dividend = static_cast<int16_t>(emulator->getRegisters().AX.x);
        emulator->getRegisters().AX.bytes.l = dividend / divisor;
        emulator->getRegisters().AX.bytes.h = dividend % divisor;
    } else {
        int16_t divisor = static_cast<int16_t>(emulator->getValue(operands[0]));
        if (divisor == 0)
            throw std::runtime_error("Division by zero");
        int32_t dividend = (emulator->getRegisters().DX.x << 16) | emulator->getRegisters().AX.x;
        emulator->getRegisters().AX.x = dividend / divisor;
        emulator->getRegisters().DX.x = dividend % divisor;
    }
}

void ArithmeticInstructions::aad(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("AAD takes no operands");
    emulator->getRegisters().AX.bytes.l =
        (emulator->getRegisters().AX.bytes.h * 10) + emulator->getRegisters().AX.bytes.l;
    emulator->getRegisters().AX.bytes.h = 0;
    emulator->updateFlags(emulator->getRegisters().AX.x, false, false);
}

void ArithmeticInstructions::cbw(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CBW takes no operands");
    emulator->getRegisters().AX.bytes.h =
        (emulator->getRegisters().AX.bytes.l & 0x80) ? 0xFF : 0x00;
}

void ArithmeticInstructions::cwd(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CWD takes no operands");
    emulator->getRegisters().DX.x = (emulator->getRegisters().AX.x & 0x8000) ? 0xFFFF : 0x0000;
}
