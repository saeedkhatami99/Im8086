#include "instructions/string.h"
#include "emulator8086.h"
#include <stdexcept>
#include <algorithm>

StringInstructions::StringInstructions(Emulator8086 *emu) : emulator(emu) {}

void StringInstructions::movsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("MOVSB takes no operands");
    uint8_t value = emulator->readMemoryByte(emulator->getRegisters().SI);
    emulator->writeMemoryByte(emulator->getRegisters().DI, value);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -1 : 1;
    emulator->getRegisters().SI += adjust;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::movsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("MOVSW takes no operands");
    uint16_t value = emulator->readMemoryWord(emulator->getRegisters().SI);
    emulator->writeMemoryWord(emulator->getRegisters().DI, value);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -2 : 2;
    emulator->getRegisters().SI += adjust;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::cmpsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMPSB takes no operands");
    uint8_t src = emulator->readMemoryByte(emulator->getRegisters().SI);
    uint8_t dest = emulator->readMemoryByte(emulator->getRegisters().DI);
    uint16_t result = src - dest;
    emulator->updateFlags(result, true, true);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -1 : 1;
    emulator->getRegisters().SI += adjust;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::cmpsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMPSW takes no operands");
    uint16_t src = emulator->readMemoryWord(emulator->getRegisters().SI);
    uint16_t dest = emulator->readMemoryWord(emulator->getRegisters().DI);
    uint32_t result = src - dest;
    emulator->updateFlags(result, false, true);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -2 : 2;
    emulator->getRegisters().SI += adjust;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::scasb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SCASB takes no operands");
    uint8_t dest = emulator->getRegisters().AX.bytes.l;
    uint8_t src = emulator->readMemoryByte(emulator->getRegisters().DI);
    uint16_t result = dest - src;
    emulator->updateFlags(result, true, true);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -1 : 1;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::scasw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("SCASW takes no operands");
    uint16_t dest = emulator->getRegisters().AX.x;
    uint16_t src = emulator->readMemoryWord(emulator->getRegisters().DI);
    uint32_t result = dest - src;
    emulator->updateFlags(result, false, true);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -2 : 2;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::lodsb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LODSB takes no operands");
    emulator->getRegisters().AX.bytes.l = emulator->readMemoryByte(emulator->getRegisters().SI);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -1 : 1;
    emulator->getRegisters().SI += adjust;
}

void StringInstructions::lodsw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("LODSW takes no operands");
    emulator->getRegisters().AX.x = emulator->readMemoryWord(emulator->getRegisters().SI);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -2 : 2;
    emulator->getRegisters().SI += adjust;
}

void StringInstructions::stosb(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STOSB takes no operands");
    emulator->writeMemoryByte(emulator->getRegisters().DI, emulator->getRegisters().AX.bytes.l);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -1 : 1;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::stosw(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STOSW takes no operands");
    emulator->writeMemoryWord(emulator->getRegisters().DI, emulator->getRegisters().AX.x);
    int adjust = (emulator->getRegisters().FLAGS & Registers::DF) ? -2 : 2;
    emulator->getRegisters().DI += adjust;
}

void StringInstructions::rep(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("REP requires 1 string operation");

    std::string op = operands[0];
    std::transform(op.begin(), op.end(), op.begin(), ::toupper);

    while (emulator->getRegisters().CX.x > 0)
    {
        if (op == "MOVSB")
            movsb({});
        else if (op == "MOVSW")
            movsw({});
        else if (op == "STOSB")
            stosb({});
        else if (op == "STOSW")
            stosw({});
        else if (op == "LODSB")
            lodsb({});
        else if (op == "LODSW")
            lodsw({});
        else
            throw std::runtime_error("REP not supported for " + op);

        emulator->getRegisters().CX.x--;
    }
}

void StringInstructions::repe(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("REPE requires 1 string operation");

    std::string op = operands[0];
    std::transform(op.begin(), op.end(), op.begin(), ::toupper);

    while (emulator->getRegisters().CX.x > 0)
    {
        if (op == "CMPSB")
            cmpsb({});
        else if (op == "CMPSW")
            cmpsw({});
        else if (op == "SCASB")
            scasb({});
        else if (op == "SCASW")
            scasw({});
        else
            throw std::runtime_error("REPE not supported for " + op);

        emulator->getRegisters().CX.x--;

        if (!(emulator->getRegisters().FLAGS & Registers::ZF))
            break;
    }
}

void StringInstructions::repne(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("REPNE requires 1 string operation");

    std::string op = operands[0];
    std::transform(op.begin(), op.end(), op.begin(), ::toupper);

    while (emulator->getRegisters().CX.x > 0)
    {
        if (op == "CMPSB")
            cmpsb({});
        else if (op == "CMPSW")
            cmpsw({});
        else if (op == "SCASB")
            scasb({});
        else if (op == "SCASW")
            scasw({});
        else
            throw std::runtime_error("REPNE not supported for " + op);

        emulator->getRegisters().CX.x--;

        if (emulator->getRegisters().FLAGS & Registers::ZF)
            break;
    }
}

void StringInstructions::repnz(const std::vector<std::string> &operands)
{
    repne(operands);
}

void StringInstructions::repz(const std::vector<std::string> &operands)
{
    repe(operands);
}

void StringInstructions::xlat(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("XLAT takes no operands");
    uint16_t address = emulator->getRegisters().BX.x + emulator->getRegisters().AX.bytes.l;
    emulator->getRegisters().AX.bytes.l = emulator->readMemoryByte(address);
}

void StringInstructions::xlatb(const std::vector<std::string> &operands)
{
    xlat(operands);
}
