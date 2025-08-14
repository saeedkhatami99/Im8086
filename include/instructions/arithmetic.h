#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086;

class ArithmeticInstructions
{
private:
    Emulator8086 *emulator;

public:
    ArithmeticInstructions(Emulator8086 *emu);

    void add(const std::vector<std::string> &operands);
    void adc(const std::vector<std::string> &operands);
    void inc(const std::vector<std::string> &operands);
    void aaa(const std::vector<std::string> &operands);
    void daa(const std::vector<std::string> &operands);
    void sub(const std::vector<std::string> &operands);
    void sbb(const std::vector<std::string> &operands);
    void dec(const std::vector<std::string> &operands);
    void neg(const std::vector<std::string> &operands);
    void aas(const std::vector<std::string> &operands);
    void das(const std::vector<std::string> &operands);
    void mul(const std::vector<std::string> &operands);
    void imul(const std::vector<std::string> &operands);
    void aam(const std::vector<std::string> &operands);
    void div(const std::vector<std::string> &operands);
    void idiv(const std::vector<std::string> &operands);
    void aad(const std::vector<std::string> &operands);
    void cbw(const std::vector<std::string> &operands);
    void cwd(const std::vector<std::string> &operands);
};

#endif
