#include "instructions/program_transfer.h"
#include "emulator8086.h"
#include <stdexcept>

ProgramTransferInstructions::ProgramTransferInstructions(Emulator8086 *emu) : emulator(emu) {}

void ProgramTransferInstructions::call(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("CALL requires 1 operand");

    emulator->getRegisters().SP -= 2;
    emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().IP);

    if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
    {
        emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
    }
    else
    {
        throw std::runtime_error("Unknown label: " + operands[0]);
    }
}

void ProgramTransferInstructions::jmp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JMP requires 1 operand");

    if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
    {
        emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
    }
    else
    {
        throw std::runtime_error("Unknown label: " + operands[0]);
    }
}

void ProgramTransferInstructions::ret(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("RET takes no operands");

    emulator->getRegisters().IP = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
}

void ProgramTransferInstructions::retf(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("RETF takes no operands");

    emulator->getRegisters().IP = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    emulator->getRegisters().CS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
}

void ProgramTransferInstructions::je(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JE requires 1 operand");

    if (emulator->getRegisters().FLAGS & Registers::ZF)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jl(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JL requires 1 operand");

    bool sf = emulator->getRegisters().FLAGS & Registers::SF;
    bool of = emulator->getRegisters().FLAGS & Registers::OF;
    if (sf != of)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jle(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JLE requires 1 operand");

    bool sf = emulator->getRegisters().FLAGS & Registers::SF;
    bool of = emulator->getRegisters().FLAGS & Registers::OF;
    bool zf = emulator->getRegisters().FLAGS & Registers::ZF;
    if ((sf != of) || zf)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jb(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JB requires 1 operand");

    if (emulator->getRegisters().FLAGS & Registers::CF)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jbe(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JBE requires 1 operand");

    if ((emulator->getRegisters().FLAGS & Registers::CF) || (emulator->getRegisters().FLAGS & Registers::ZF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JP requires 1 operand");

    if (emulator->getRegisters().FLAGS & Registers::PF)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jo(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JO requires 1 operand");

    if (emulator->getRegisters().FLAGS & Registers::OF)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::js(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JS requires 1 operand");

    if (emulator->getRegisters().FLAGS & Registers::SF)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jne(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNE requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::ZF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jnl(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNL requires 1 operand");

    bool sf = emulator->getRegisters().FLAGS & Registers::SF;
    bool of = emulator->getRegisters().FLAGS & Registers::OF;
    if (sf == of)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jg(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JG requires 1 operand");

    bool sf = emulator->getRegisters().FLAGS & Registers::SF;
    bool of = emulator->getRegisters().FLAGS & Registers::OF;
    bool zf = emulator->getRegisters().FLAGS & Registers::ZF;
    if ((sf == of) && !zf)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jnb(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNB requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::CF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::ja(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JA requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::CF) && !(emulator->getRegisters().FLAGS & Registers::ZF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jnp(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNP requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::PF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jno(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNO requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::OF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jns(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JNS requires 1 operand");

    if (!(emulator->getRegisters().FLAGS & Registers::SF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::loop(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOP requires 1 operand");

    emulator->getRegisters().CX.x--;
    if (emulator->getRegisters().CX.x != 0)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::loopz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOPZ requires 1 operand");

    emulator->getRegisters().CX.x--;
    if (emulator->getRegisters().CX.x != 0 && (emulator->getRegisters().FLAGS & Registers::ZF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::loopnz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("LOOPNZ requires 1 operand");

    emulator->getRegisters().CX.x--;
    if (emulator->getRegisters().CX.x != 0 && !(emulator->getRegisters().FLAGS & Registers::ZF))
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}

void ProgramTransferInstructions::jcxz(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("JCXZ requires 1 operand");

    if (emulator->getRegisters().CX.x == 0)
    {
        if (emulator->getLabels().find(operands[0]) != emulator->getLabels().end())
        {
            emulator->getRegisters().IP = emulator->getLabels()[operands[0]];
        }
        else
        {
            throw std::runtime_error("Unknown label: " + operands[0]);
        }
    }
}
