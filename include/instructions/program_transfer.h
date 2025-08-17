#ifndef PROGRAM_TRANSFER_H
#define PROGRAM_TRANSFER_H

#include <string>
#include <vector>

#include "../memory_components.h"
#include "../registers.h"

class Emulator8086;

class ProgramTransferInstructions {
  private:
    Emulator8086* emulator;

  public:
    ProgramTransferInstructions(Emulator8086* emu);

    void call(const std::vector<std::string>& operands);
    void jmp(const std::vector<std::string>& operands);
    void ret(const std::vector<std::string>& operands);
    void retf(const std::vector<std::string>& operands);

    void je(const std::vector<std::string>& operands);
    void jl(const std::vector<std::string>& operands);
    void jle(const std::vector<std::string>& operands);
    void jb(const std::vector<std::string>& operands);
    void jbe(const std::vector<std::string>& operands);
    void jp(const std::vector<std::string>& operands);
    void jo(const std::vector<std::string>& operands);
    void js(const std::vector<std::string>& operands);
    void jne(const std::vector<std::string>& operands);
    void jnl(const std::vector<std::string>& operands);
    void jg(const std::vector<std::string>& operands);
    void jnb(const std::vector<std::string>& operands);
    void ja(const std::vector<std::string>& operands);
    void jnp(const std::vector<std::string>& operands);
    void jno(const std::vector<std::string>& operands);
    void jns(const std::vector<std::string>& operands);

    void loop(const std::vector<std::string>& operands);
    void loopz(const std::vector<std::string>& operands);
    void loopnz(const std::vector<std::string>& operands);
    void jcxz(const std::vector<std::string>& operands);
};

#endif
