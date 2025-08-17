#include "emulator8086.h"

#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "instructions/arithmetic.h"
#include "instructions/bit_manipulation.h"
#include "instructions/data_transfer.h"
#include "instructions/logical.h"
#include "instructions/processor_control.h"
#include "instructions/program_transfer.h"
#include "instructions/string.h"

Emulator8086::Emulator8086(size_t memSize) : memory(memSize, 0) {
    dataTransfer = std::make_unique<DataTransferInstructions>(this);
    arithmetic = std::make_unique<ArithmeticInstructions>(this);
    logical = std::make_unique<LogicalInstructions>(this);
    string = std::make_unique<StringInstructions>(this);
    programTransfer = std::make_unique<ProgramTransferInstructions>(this);
    processorControl = std::make_unique<ProcessorControlInstructions>(this);
    bitManipulation = std::make_unique<BitManipulationInstructions>(this);

    initializeInstructions();
}

Emulator8086::~Emulator8086() = default;

void Emulator8086::initializeInstructions() {
    instructions["MOV"] = [this](std::vector<std::string>& operands) {
        dataTransfer->mov(operands);
    };
    instructions["PUSH"] = [this](std::vector<std::string>& operands) {
        dataTransfer->push(operands);
    };
    instructions["POP"] = [this](std::vector<std::string>& operands) {
        dataTransfer->pop(operands);
    };
    instructions["XCHG"] = [this](std::vector<std::string>& operands) {
        dataTransfer->xchg(operands);
    };
    instructions["LEA"] = [this](std::vector<std::string>& operands) {
        dataTransfer->lea(operands);
    };
    instructions["LDS"] = [this](std::vector<std::string>& operands) {
        dataTransfer->lds(operands);
    };
    instructions["LES"] = [this](std::vector<std::string>& operands) {
        dataTransfer->les(operands);
    };
    instructions["LAHF"] = [this](std::vector<std::string>& operands) {
        dataTransfer->lahf(operands);
    };
    instructions["SAHF"] = [this](std::vector<std::string>& operands) {
        dataTransfer->sahf(operands);
    };
    instructions["PUSHF"] = [this](std::vector<std::string>& operands) {
        dataTransfer->pushf(operands);
    };
    instructions["POPF"] = [this](std::vector<std::string>& operands) {
        dataTransfer->popf(operands);
    };
    instructions["PUSHA"] = [this](std::vector<std::string>& operands) {
        dataTransfer->pusha(operands);
    };
    instructions["POPA"] = [this](std::vector<std::string>& operands) {
        dataTransfer->popa(operands);
    };

    instructions["ADD"] = [this](std::vector<std::string>& operands) { arithmetic->add(operands); };
    instructions["ADC"] = [this](std::vector<std::string>& operands) { arithmetic->adc(operands); };
    instructions["INC"] = [this](std::vector<std::string>& operands) { arithmetic->inc(operands); };
    instructions["AAA"] = [this](std::vector<std::string>& operands) { arithmetic->aaa(operands); };
    instructions["DAA"] = [this](std::vector<std::string>& operands) { arithmetic->daa(operands); };
    instructions["SUB"] = [this](std::vector<std::string>& operands) { arithmetic->sub(operands); };
    instructions["SBB"] = [this](std::vector<std::string>& operands) { arithmetic->sbb(operands); };
    instructions["DEC"] = [this](std::vector<std::string>& operands) { arithmetic->dec(operands); };
    instructions["NEG"] = [this](std::vector<std::string>& operands) { arithmetic->neg(operands); };
    instructions["AAS"] = [this](std::vector<std::string>& operands) { arithmetic->aas(operands); };
    instructions["DAS"] = [this](std::vector<std::string>& operands) { arithmetic->das(operands); };
    instructions["MUL"] = [this](std::vector<std::string>& operands) { arithmetic->mul(operands); };
    instructions["IMUL"] = [this](std::vector<std::string>& operands) {
        arithmetic->imul(operands);
    };
    instructions["AAM"] = [this](std::vector<std::string>& operands) { arithmetic->aam(operands); };
    instructions["DIV"] = [this](std::vector<std::string>& operands) { arithmetic->div(operands); };
    instructions["IDIV"] = [this](std::vector<std::string>& operands) {
        arithmetic->idiv(operands);
    };
    instructions["AAD"] = [this](std::vector<std::string>& operands) { arithmetic->aad(operands); };
    instructions["CBW"] = [this](std::vector<std::string>& operands) { arithmetic->cbw(operands); };
    instructions["CWD"] = [this](std::vector<std::string>& operands) { arithmetic->cwd(operands); };

    instructions["AND"] = [this](std::vector<std::string>& operands) { logical->and_op(operands); };
    instructions["OR"] = [this](std::vector<std::string>& operands) { logical->or_op(operands); };
    instructions["XOR"] = [this](std::vector<std::string>& operands) { logical->xor_op(operands); };
    instructions["NOT"] = [this](std::vector<std::string>& operands) { logical->not_op(operands); };
    instructions["TEST"] = [this](std::vector<std::string>& operands) { logical->test(operands); };
    instructions["CMP"] = [this](std::vector<std::string>& operands) { logical->cmp(operands); };

    instructions["MOVSB"] = [this](std::vector<std::string>& operands) { string->movsb(operands); };
    instructions["MOVSW"] = [this](std::vector<std::string>& operands) { string->movsw(operands); };
    instructions["CMPSB"] = [this](std::vector<std::string>& operands) { string->cmpsb(operands); };
    instructions["CMPSW"] = [this](std::vector<std::string>& operands) { string->cmpsw(operands); };
    instructions["SCASB"] = [this](std::vector<std::string>& operands) { string->scasb(operands); };
    instructions["SCASW"] = [this](std::vector<std::string>& operands) { string->scasw(operands); };
    instructions["LODSB"] = [this](std::vector<std::string>& operands) { string->lodsb(operands); };
    instructions["LODSW"] = [this](std::vector<std::string>& operands) { string->lodsw(operands); };
    instructions["STOSB"] = [this](std::vector<std::string>& operands) { string->stosb(operands); };
    instructions["STOSW"] = [this](std::vector<std::string>& operands) { string->stosw(operands); };
    instructions["REP"] = [this](std::vector<std::string>& operands) { string->rep(operands); };
    instructions["REPE"] = [this](std::vector<std::string>& operands) { string->repe(operands); };
    instructions["REPNE"] = [this](std::vector<std::string>& operands) { string->repne(operands); };
    instructions["REPNZ"] = [this](std::vector<std::string>& operands) { string->repnz(operands); };
    instructions["REPZ"] = [this](std::vector<std::string>& operands) { string->repz(operands); };
    instructions["XLAT"] = [this](std::vector<std::string>& operands) { string->xlat(operands); };
    instructions["XLATB"] = [this](std::vector<std::string>& operands) { string->xlatb(operands); };

    instructions["CALL"] = [this](std::vector<std::string>& operands) {
        programTransfer->call(operands);
    };
    instructions["JMP"] = [this](std::vector<std::string>& operands) {
        programTransfer->jmp(operands);
    };
    instructions["RET"] = [this](std::vector<std::string>& operands) {
        programTransfer->ret(operands);
    };
    instructions["RETF"] = [this](std::vector<std::string>& operands) {
        programTransfer->retf(operands);
    };
    instructions["JE"] = [this](std::vector<std::string>& operands) {
        programTransfer->je(operands);
    };
    instructions["JZ"] = [this](std::vector<std::string>& operands) {
        programTransfer->je(operands);
    };
    instructions["JL"] = [this](std::vector<std::string>& operands) {
        programTransfer->jl(operands);
    };
    instructions["JNGE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jl(operands);
    };
    instructions["JLE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jle(operands);
    };
    instructions["JNG"] = [this](std::vector<std::string>& operands) {
        programTransfer->jle(operands);
    };
    instructions["JB"] = [this](std::vector<std::string>& operands) {
        programTransfer->jb(operands);
    };
    instructions["JNAE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jb(operands);
    };
    instructions["JC"] = [this](std::vector<std::string>& operands) {
        programTransfer->jb(operands);
    };
    instructions["JBE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jbe(operands);
    };
    instructions["JNA"] = [this](std::vector<std::string>& operands) {
        programTransfer->jbe(operands);
    };
    instructions["JP"] = [this](std::vector<std::string>& operands) {
        programTransfer->jp(operands);
    };
    instructions["JPE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jp(operands);
    };
    instructions["JO"] = [this](std::vector<std::string>& operands) {
        programTransfer->jo(operands);
    };
    instructions["JS"] = [this](std::vector<std::string>& operands) {
        programTransfer->js(operands);
    };
    instructions["JNE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jne(operands);
    };
    instructions["JNZ"] = [this](std::vector<std::string>& operands) {
        programTransfer->jne(operands);
    };
    instructions["JNL"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnl(operands);
    };
    instructions["JGE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnl(operands);
    };
    instructions["JG"] = [this](std::vector<std::string>& operands) {
        programTransfer->jg(operands);
    };
    instructions["JNLE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jg(operands);
    };
    instructions["JNB"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnb(operands);
    };
    instructions["JAE"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnb(operands);
    };
    instructions["JNC"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnb(operands);
    };
    instructions["JA"] = [this](std::vector<std::string>& operands) {
        programTransfer->ja(operands);
    };
    instructions["JNBE"] = [this](std::vector<std::string>& operands) {
        programTransfer->ja(operands);
    };
    instructions["JNP"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnp(operands);
    };
    instructions["JPO"] = [this](std::vector<std::string>& operands) {
        programTransfer->jnp(operands);
    };
    instructions["JNO"] = [this](std::vector<std::string>& operands) {
        programTransfer->jno(operands);
    };
    instructions["JNS"] = [this](std::vector<std::string>& operands) {
        programTransfer->jns(operands);
    };
    instructions["LOOP"] = [this](std::vector<std::string>& operands) {
        programTransfer->loop(operands);
    };
    instructions["LOOPZ"] = [this](std::vector<std::string>& operands) {
        programTransfer->loopz(operands);
    };
    instructions["LOOPE"] = [this](std::vector<std::string>& operands) {
        programTransfer->loopz(operands);
    };
    instructions["LOOPNZ"] = [this](std::vector<std::string>& operands) {
        programTransfer->loopnz(operands);
    };
    instructions["LOOPNE"] = [this](std::vector<std::string>& operands) {
        programTransfer->loopnz(operands);
    };
    instructions["JCXZ"] = [this](std::vector<std::string>& operands) {
        programTransfer->jcxz(operands);
    };

    instructions["CLC"] = [this](std::vector<std::string>& operands) {
        processorControl->clc(operands);
    };
    instructions["CMC"] = [this](std::vector<std::string>& operands) {
        processorControl->cmc(operands);
    };
    instructions["STC"] = [this](std::vector<std::string>& operands) {
        processorControl->stc(operands);
    };
    instructions["CLD"] = [this](std::vector<std::string>& operands) {
        processorControl->cld(operands);
    };
    instructions["STD"] = [this](std::vector<std::string>& operands) {
        processorControl->std(operands);
    };
    instructions["CLI"] = [this](std::vector<std::string>& operands) {
        processorControl->cli(operands);
    };
    instructions["STI"] = [this](std::vector<std::string>& operands) {
        processorControl->sti(operands);
    };
    instructions["HLT"] = [this](std::vector<std::string>& operands) {
        processorControl->hlt(operands);
    };
    instructions["WAIT"] = [this](std::vector<std::string>& operands) {
        processorControl->wait(operands);
    };
    instructions["ESC"] = [this](std::vector<std::string>& operands) {
        processorControl->esc(operands);
    };
    instructions["LOCK"] = [this](std::vector<std::string>& operands) {
        processorControl->lock(operands);
    };
    instructions["NOP"] = [this](std::vector<std::string>& operands) {
        processorControl->nop(operands);
    };
    instructions["INT"] = [this](std::vector<std::string>& operands) {
        processorControl->int_op(operands);
    };
    instructions["INTO"] = [this](std::vector<std::string>& operands) {
        processorControl->into(operands);
    };
    instructions["IRET"] = [this](std::vector<std::string>& operands) {
        processorControl->iret(operands);
    };
    instructions["IN"] = [this](std::vector<std::string>& operands) {
        processorControl->in_op(operands);
    };
    instructions["OUT"] = [this](std::vector<std::string>& operands) {
        processorControl->out(operands);
    };

    instructions["RCL"] = [this](std::vector<std::string>& operands) {
        bitManipulation->rcl(operands);
    };
    instructions["RCR"] = [this](std::vector<std::string>& operands) {
        bitManipulation->rcr(operands);
    };
    instructions["ROL"] = [this](std::vector<std::string>& operands) {
        bitManipulation->rol(operands);
    };
    instructions["ROR"] = [this](std::vector<std::string>& operands) {
        bitManipulation->ror(operands);
    };
    instructions["SAL"] = [this](std::vector<std::string>& operands) {
        bitManipulation->sal(operands);
    };
    instructions["SAR"] = [this](std::vector<std::string>& operands) {
        bitManipulation->sar(operands);
    };
    instructions["SHL"] = [this](std::vector<std::string>& operands) {
        bitManipulation->shl(operands);
    };
    instructions["SHR"] = [this](std::vector<std::string>& operands) {
        bitManipulation->shr(operands);
    };
}

uint16_t& Emulator8086::getRegister(const std::string& reg) {
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

uint8_t& Emulator8086::getRegister8(const std::string& reg) {
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);
    if (upperReg == "AL")
        return regs.AX.bytes.l;
    if (upperReg == "AH")
        return regs.AX.bytes.h;
    if (upperReg == "BL")
        return regs.BX.bytes.l;
    if (upperReg == "BH")
        return regs.BX.bytes.h;
    if (upperReg == "CL")
        return regs.CX.bytes.l;
    if (upperReg == "CH")
        return regs.CX.bytes.h;
    if (upperReg == "DL")
        return regs.DX.bytes.l;
    if (upperReg == "DH")
        return regs.DX.bytes.h;
    throw std::runtime_error("Invalid 8-bit register: " + reg);
}

bool Emulator8086::is8BitRegister(const std::string& reg) {
    std::string upperReg = reg;
    std::transform(upperReg.begin(), upperReg.end(), upperReg.begin(), ::toupper);
    return (upperReg == "AL" || upperReg == "AH" || upperReg == "BL" || upperReg == "BH" ||
            upperReg == "CL" || upperReg == "CH" || upperReg == "DL" || upperReg == "DH");
}

bool Emulator8086::isMemoryOperand(const std::string& operand) {
    return operand[0] == '[' && operand.back() == ']';
}

MemoryOperand Emulator8086::parseMemoryOperand(const std::string& operand) {
    MemoryOperand result;
    std::string inner = operand.substr(1, operand.length() - 2);
    std::vector<std::string> parts;
    std::string current;
    bool negative = false;

    for (size_t i = 0; i < inner.length(); i++) {
        if (inner[i] == '+' || inner[i] == '-') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            if (inner[i] == '-')
                negative = true;
        } else {
            current += inner[i];
        }
    }
    if (!current.empty())
        parts.push_back(current);

    for (const auto& part : parts) {
        std::string upperPart = part;
        std::transform(upperPart.begin(), upperPart.end(), upperPart.begin(), ::toupper);
        if (upperPart == "BX") {
            result.hasBase = true;
            result.base = regs.BX.x;
        } else if (upperPart == "BP") {
            result.hasBase = true;
            result.base = regs.BP;
        } else if (upperPart == "SI") {
            result.hasIndex = true;
            result.index = regs.SI;
        } else if (upperPart == "DI") {
            result.hasIndex = true;
            result.index = regs.DI;
        } else {
            result.hasDisplacement = true;
            try {
                result.displacement = std::stoi(part, nullptr, 16);
                if (negative)
                    result.displacement = -result.displacement;
            } catch (const std::invalid_argument& e) {
                throw std::runtime_error("Invalid displacement value: " + part);
            } catch (const std::out_of_range& e) {
                throw std::runtime_error("Displacement value out of range: " + part);
            }
        }
    }
    return result;
}

uint16_t Emulator8086::calculateEffectiveAddress(const MemoryOperand& memOp) {
    uint16_t ea = 0;
    if (memOp.hasBase)
        ea += memOp.base;
    if (memOp.hasIndex)
        ea += memOp.index;
    if (memOp.hasDisplacement)
        ea += memOp.displacement;
    return ea;
}

uint16_t Emulator8086::readMemoryWord(uint16_t address) {
    if (static_cast<size_t>(address) + 1 >= memory.size())
        throw std::out_of_range("Memory address out of range");
    return (memory[address + 1] << 8) | memory[address];
}

void Emulator8086::writeMemoryWord(uint16_t address, uint16_t value) {
    if (static_cast<size_t>(address) + 1 >= memory.size())
        throw std::out_of_range("Memory address out of range");
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

uint8_t Emulator8086::readMemoryByte(uint16_t address) {
    if (static_cast<size_t>(address) >= memory.size())
        throw std::out_of_range("Memory address out of range");
    return memory[address];
}

void Emulator8086::writeMemoryByte(uint16_t address, uint8_t value) {
    if (static_cast<size_t>(address) >= memory.size())
        throw std::out_of_range("Memory address out of range");
    memory[address] = value;
}

uint16_t Emulator8086::getValue(const std::string& operand) {
    if (isMemoryOperand(operand)) {
        MemoryOperand memOp = parseMemoryOperand(operand);
        uint16_t address = calculateEffectiveAddress(memOp);
        return readMemoryWord(address);
    } else if (operand.back() == 'h') {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16);
    } else if (operand[0] >= '0' && operand[0] <= '9') {
        try {
            return std::stoi(operand);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid immediate value: " + operand);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Immediate value out of range: " + operand);
        }
    } else if (is8BitRegister(operand)) {
        throw std::runtime_error("Cannot get 16-bit value from 8-bit register");
    } else {
        return getRegister(operand);
    }
}

uint8_t Emulator8086::getValue8(const std::string& operand) {
    if (isMemoryOperand(operand)) {
        MemoryOperand memOp = parseMemoryOperand(operand);
        uint16_t address = calculateEffectiveAddress(memOp);
        return readMemoryByte(address);
    } else if (operand.back() == 'h') {
        std::string hexValue = operand.substr(0, operand.size() - 1);
        return std::stoul(hexValue, nullptr, 16) & 0xFF;
    } else if (operand[0] >= '0' && operand[0] <= '9') {
        try {
            return std::stoi(operand) & 0xFF;
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid immediate value: " + operand);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Immediate value out of range: " + operand);
        }
    } else if (is8BitRegister(operand)) {
        return getRegister8(operand);
    } else {
        throw std::runtime_error("Invalid operand for 8-bit value");
    }
}

void Emulator8086::updateFlags(uint32_t result, bool isByte, bool checkCarry) {
    uint16_t mask = isByte ? 0xFF : 0xFFFF;
    uint16_t res = result & mask;

    regs.FLAGS = (res == 0) ? (regs.FLAGS | Registers::ZF) : (regs.FLAGS & ~Registers::ZF);

    regs.FLAGS = (isByte ? (res & 0x80) : (res & 0x8000)) ? (regs.FLAGS | Registers::SF)
                                                          : (regs.FLAGS & ~Registers::SF);

    if (checkCarry) {
        regs.FLAGS = (isByte ? (result > 0xFF) : (result > 0xFFFF)) ? (regs.FLAGS | Registers::CF)
                                                                    : (regs.FLAGS & ~Registers::CF);
    }

    uint8_t leastByte = res & 0xFF;
    uint8_t parity = 0;
    for (int i = 0; i < 8; ++i)
        parity ^= (leastByte >> i) & 1;
    regs.FLAGS = (parity == 0) ? (regs.FLAGS | Registers::PF) : (regs.FLAGS & ~Registers::PF);
}

void Emulator8086::executeInstruction(const std::string& instruction) {
    std::istringstream iss(instruction);
    std::string mnemonic;
    iss >> mnemonic;

    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

    std::vector<std::string> operands;
    std::string operand;
    while (std::getline(iss, operand, ',')) {
        operand.erase(operand.begin(), std::find_if(operand.begin(), operand.end(), [](int ch) {
                          return !std::isspace(ch);
                      }));
        operand.erase(
            std::find_if(operand.rbegin(), operand.rend(), [](int ch) { return !std::isspace(ch); })
                .base(),
            operand.end());
        if (!operand.empty())
            operands.push_back(operand);
    }

    auto it = instructions.find(mnemonic);
    if (it != instructions.end()) {
        it->second(operands);
    } else {
        throw std::runtime_error("Unknown instruction: " + mnemonic);
    }
}

void Emulator8086::loadProgram(const std::vector<std::string>& lines) {
    program.clear();
    labels.clear();
    regs.IP = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];

        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
                       return !std::isspace(ch);
                   }));
        line.erase(
            std::find_if(line.rbegin(), line.rend(), [](int ch) { return !std::isspace(ch); })
                .base(),
            line.end());
        if (line.empty())
            continue;

        auto scPos = line.find(';');
        if (scPos != std::string::npos)
            line = line.substr(0, scPos);
        if (line.empty())
            continue;
        if (line.back() == ':') {
            std::string label = line.substr(0, line.size() - 1);
            std::transform(label.begin(), label.end(), label.begin(), ::toupper);
            labels[label] = program.size();
        } else {
            program.push_back(line);
        }
    }
}

bool Emulator8086::step() {
    if (regs.IP >= program.size())
        return false;
    std::string instr = program[regs.IP];
    size_t oldIP = regs.IP;
    try {
        executeInstruction(instr);
    } catch (const std::exception& e) {
        std::cerr << "Execution error at IP=" << oldIP << ": " << e.what() << "\n";
    }

    if (regs.IP == oldIP)
        regs.IP++;
    return regs.IP < program.size();
}

void Emulator8086::reset() {
    regs = Registers();
    std::fill(memory.begin(), memory.end(), 0);
}

void Emulator8086::displayRegisters() {
    std::cout << std::hex << std::uppercase << std::setfill('0') << "AX=" << std::setw(4)
              << regs.AX.x << " (AH=" << std::setw(2) << static_cast<uint16_t>(regs.AX.bytes.h)
              << ", AL=" << std::setw(2) << static_cast<uint16_t>(regs.AX.bytes.l) << ")\n"
              << "BX=" << std::setw(4) << regs.BX.x << " (BH=" << std::setw(2)
              << static_cast<uint16_t>(regs.BX.bytes.h) << ", BL=" << std::setw(2)
              << static_cast<uint16_t>(regs.BX.bytes.l) << ")\n"
              << "CX=" << std::setw(4) << regs.CX.x << " (CH=" << std::setw(2)
              << static_cast<uint16_t>(regs.CX.bytes.h) << ", CL=" << std::setw(2)
              << static_cast<uint16_t>(regs.CX.bytes.l) << ")\n"
              << "DX=" << std::setw(4) << regs.DX.x << " (DH=" << std::setw(2)
              << static_cast<uint16_t>(regs.DX.bytes.h) << ", DL=" << std::setw(2)
              << static_cast<uint16_t>(regs.DX.bytes.l) << ")\n"
              << "SI=" << std::setw(4) << regs.SI << "  "
              << "DI=" << std::setw(4) << regs.DI << "  "
              << "BP=" << std::setw(4) << regs.BP << "  "
              << "SP=" << std::setw(4) << regs.SP << "\n"
              << "CS=" << std::setw(4) << regs.CS << "  "
              << "DS=" << std::setw(4) << regs.DS << "  "
              << "ES=" << std::setw(4) << regs.ES << "  "
              << "SS=" << std::setw(4) << regs.SS << "\n"
              << "IP=" << std::setw(4) << regs.IP << "  "
              << "FLAGS=" << std::setw(4) << regs.FLAGS << " ["
              << (regs.FLAGS & Registers::OF ? "O" : "-")
              << (regs.FLAGS & Registers::DF ? "D" : "-")
              << (regs.FLAGS & Registers::IF ? "I" : "-")
              << (regs.FLAGS & Registers::TF ? "T" : "-")
              << (regs.FLAGS & Registers::SF ? "S" : "-")
              << (regs.FLAGS & Registers::ZF ? "Z" : "-")
              << (regs.FLAGS & Registers::AF ? "A" : "-")
              << (regs.FLAGS & Registers::PF ? "P" : "-")
              << (regs.FLAGS & Registers::CF ? "C" : "-") << "]\n";
}

void Emulator8086::displayStack() {
    std::cout << "\nStack contents:\n";
    if (regs.SP == 0xFFFE) {
        std::cout << "Stack is empty\n";
        return;
    }
    for (uint16_t i = regs.SP; i < 0xFFFE; i += 2) {
        std::cout << std::hex << std::uppercase << std::setfill('0') << "SP+" << std::setw(4)
                  << (i - regs.SP) << ": " << std::setw(4) << readMemoryWord(i) << '\n';
    }
}

void Emulator8086::displayMemory(uint16_t address, uint16_t count) {
    std::cout << "\nMemory dump from " << std::hex << std::uppercase << std::setfill('0')
              << std::setw(4) << address << ":\n";
    for (uint16_t i = 0; i < count; i++) {
        if (i % 16 == 0)
            std::cout << std::setw(4) << (address + i) << ": ";
        std::cout << std::setw(2) << static_cast<int>(readMemoryByte(address + i)) << ' ';
        if (i % 16 == 15 || i == count - 1)
            std::cout << '\n';
    }
}

void Emulator8086::displayHelp() {
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
    std::cout << "  IN dest,port       - Input from port (enhanced simulation)\n";
    std::cout << "  OUT port,src       - Output to port (enhanced simulation)\n";
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
    std::cout << "  REP inst           - Repeat string instruction (enhanced)\n";
    std::cout << "  REPE/REPZ inst     - Repeat while equal (enhanced)\n";
    std::cout << "  REPNE/REPNZ inst   - Repeat while not equal (enhanced)\n";

    std::cout << "\nControl Transfer Instructions:\n";
    std::cout << "  CALL addr          - Call subroutine\n";
    std::cout << "  JMP addr           - Unconditional jump\n";
    std::cout << "  RET                - Return from subroutine\n";
    std::cout << "  RETF               - Return far (pops CS and IP)\n";
    std::cout << "  JE/JZ addr         - Jump if equal/zero (ZF=1)\n";
    std::cout << "  JL/JNGE addr       - Jump if less/not greater or equal (SF!=OF)\n";
    std::cout << "  JLE/JNG addr       - Jump if less or equal/not greater (ZF=1 or SF!=OF)\n";
    std::cout << "  JB/JNAE/JC addr    - Jump if below/not above or equal/carry (CF=1)\n";
    std::cout << "  JBE/JNA addr       - Jump if below or equal/not above (CF=1 or ZF=1)\n";
    std::cout << "  JP/JPE addr        - Jump if parity/parity even (PF=1)\n";
    std::cout << "  JO addr            - Jump if overflow (OF=1)\n";
    std::cout << "  JS addr            - Jump if sign (SF=1)\n";
    std::cout << "  JNE/JNZ addr       - Jump if not equal/not zero (ZF=0)\n";
    std::cout << "  JNL/JGE addr       - Jump if not less/greater or equal (SF=OF)\n";
    std::cout << "  JG/JNLE addr       - Jump if greater/not less or equal (ZF=0 and SF=OF)\n";
    std::cout << "  JNB/JAE/JNC addr   - Jump if not below/above or equal/not carry (CF=0)\n";
    std::cout << "  JA/JNBE addr       - Jump if above/not below or equal (CF=0 and ZF=0)\n";
    std::cout << "  JNP/JPO addr       - Jump if not parity/parity odd (PF=0)\n";
    std::cout << "  JNO addr           - Jump if no overflow (OF=0)\n";
    std::cout << "  JNS addr           - Jump if not sign (SF=0)\n";
    std::cout << "  LOOP addr          - Loop while CX != 0\n";
    std::cout << "  LOOPZ/LOOPE addr   - Loop while CX != 0 and ZF=1\n";
    std::cout << "  LOOPNZ/LOOPNE addr - Loop while CX != 0 and ZF=0\n";
    std::cout << "  JCXZ addr          - Jump if CX = 0\n";

    std::cout << "\nInterrupt Instructions:\n";
    std::cout << "  INT num            - Software interrupt (enhanced simulation)\n";
    std::cout << "  INTO               - Interrupt on overflow (enhanced)\n";
    std::cout << "  IRET               - Return from interrupt (enhanced)\n";

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
    std::cout << "  WAIT               - Wait for external signal\n";
    std::cout << "  ESC op             - Escape to coprocessor (enhanced simulation)\n";
    std::cout << "  LOCK inst          - Lock bus (simulation)\n";
    std::cout << "  NOP                - No operation\n";

    std::cout << "\nEmulator Commands:\n";
    std::cout << "  reg                - Display registers\n";
    std::cout << "  stack              - Display stack contents\n";
    std::cout << "  mem addr count     - Display memory (e.g., 'mem 100h 20h')\n";
    std::cout << "  ?                  - Show this help\n";
    std::cout << "  :3xit              - Exit emulator\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Notes: Use 'h' suffix for hex (e.g., 10h), memory as [BX+SI+offset]\n";
}

size_t Emulator8086::getLabelAddress(const std::string& label) {
    std::string upperLabel = label;
    std::transform(upperLabel.begin(), upperLabel.end(), upperLabel.begin(), ::toupper);

    auto it = labels.find(upperLabel);
    if (it != labels.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Unknown label: " + label);
    }
}

bool Emulator8086::hasLabel(const std::string& label) {
    std::string upperLabel = label;
    std::transform(upperLabel.begin(), upperLabel.end(), upperLabel.begin(), ::toupper);
    return labels.find(upperLabel) != labels.end();
}
