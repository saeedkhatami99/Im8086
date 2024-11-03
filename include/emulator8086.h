#ifndef EMULATOR8086_H
#define EMULATOR8086_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "registers.h"
#include "memory_components.h"

class Emulator8086
{
private:
    Registers regs;
    std::vector<uint8_t> memory;

    std::map<std::string, std::function<void(std::vector<std::string> &)>> instructions;
    std::map<std::string, size_t> labels;
    std::vector<std::string> program;

    void initializeInstructions();
    uint16_t &getRegister(const std::string &reg);
    uint8_t &getRegister8(const std::string &reg);
    bool is8BitRegister(const std::string &reg);
    bool isMemoryOperand(const std::string &operand);
    uint16_t getValue(const std::string &operand);
    uint8_t getValue8(const std::string &operand);
    void updateFlags(uint16_t result, bool checkCarry = true);
    MemoryOperand parseMemoryOperand(const std::string &operand);
    uint16_t calculateEffectiveAddress(const MemoryOperand &memOp);
    uint16_t readMemoryWord(uint16_t address);
    void writeMemoryWord(uint16_t address, uint16_t value);
    uint8_t readMemoryByte(uint16_t address);
    void writeMemoryByte(uint16_t address, uint8_t value);

    void mov(std::vector<std::string> &operands);
    void push(std::vector<std::string> &operands);
    void pop(std::vector<std::string> &operands);
    void xchg(std::vector<std::string> &operands);
    void lea(std::vector<std::string> &operands);
    void lds(std::vector<std::string> &operands);
    void les(std::vector<std::string> &operands);
    void lahf(std::vector<std::string> &operands);
    void sahf(std::vector<std::string> &operands);
    void pushf(std::vector<std::string> &operands);
    void popf(std::vector<std::string> &operands);
    void pusha(std::vector<std::string> &operands);
    void popa(std::vector<std::string> &operands);
    void add(std::vector<std::string> &operands);
    void adc(std::vector<std::string> &operands);
    void inc(std::vector<std::string> &operands);
    void aaa(std::vector<std::string> &operands);
    void daa(std::vector<std::string> &operands);
    void sub(std::vector<std::string> &operands);
    void sbb(std::vector<std::string> &operands);
    void dec(std::vector<std::string> &operands);
    void neg(std::vector<std::string> &operands);
    void aas(std::vector<std::string> &operands);
    void das(std::vector<std::string> &operands);
    void mul(std::vector<std::string> &operands);
    void imul(std::vector<std::string> &operands);
    void aam(std::vector<std::string> &operands);
    void div(std::vector<std::string> &operands);
    void idiv(std::vector<std::string> &operands);
    void aad(std::vector<std::string> &operands);
    void cbw(std::vector<std::string> &operands);
    void cwd(std::vector<std::string> &operands);
    void and_op(std::vector<std::string> &operands);
    void or_op(std::vector<std::string> &operands);
    void xor_op(std::vector<std::string> &operands);
    void not_op(std::vector<std::string> &operands);
    void test(std::vector<std::string> &operands);
    void cmp(std::vector<std::string> &operands);
    void movsb(std::vector<std::string> &operands);
    void movsw(std::vector<std::string> &operands);
    void cmpsb(std::vector<std::string> &operands);
    void cmpsw(std::vector<std::string> &operands);
    void scasb(std::vector<std::string> &operands);
    void scasw(std::vector<std::string> &operands);
    void lodsb(std::vector<std::string> &operands);
    void lodsw(std::vector<std::string> &operands);
    void stosb(std::vector<std::string> &operands);
    void stosw(std::vector<std::string> &operands);
    void call(std::vector<std::string> &operands);
    void jmp(std::vector<std::string> &operands);
    void ret(std::vector<std::string> &operands);
    void retf(std::vector<std::string> &operands);
    void je(std::vector<std::string> &operands);
    void jl(std::vector<std::string> &operands);
    void jle(std::vector<std::string> &operands);
    void jb(std::vector<std::string> &operands);
    void jbe(std::vector<std::string> &operands);
    void jp(std::vector<std::string> &operands);
    void jo(std::vector<std::string> &operands);
    void js(std::vector<std::string> &operands);
    void jne(std::vector<std::string> &operands);
    void jnl(std::vector<std::string> &operands);
    void jg(std::vector<std::string> &operands);
    void jnb(std::vector<std::string> &operands);
    void ja(std::vector<std::string> &operands);
    void jnp(std::vector<std::string> &operands);
    void jno(std::vector<std::string> &operands);
    void jns(std::vector<std::string> &operands);
    void loop(std::vector<std::string> &operands);
    void loopz(std::vector<std::string> &operands);
    void loopnz(std::vector<std::string> &operands);
    void jcxz(std::vector<std::string> &operands);
    void int_op(std::vector<std::string> &operands);
    void into(std::vector<std::string> &operands);
    void iret(std::vector<std::string> &operands);
    void rcl(std::vector<std::string> &operands);
    void rcr(std::vector<std::string> &operands);
    void rol(std::vector<std::string> &operands);
    void ror(std::vector<std::string> &operands);
    void sal(std::vector<std::string> &operands);
    void sar(std::vector<std::string> &operands);
    void shl(std::vector<std::string> &operands);
    void shr(std::vector<std::string> &operands);
    void clc(std::vector<std::string> &operands);
    void cmc(std::vector<std::string> &operands);
    void stc(std::vector<std::string> &operands);
    void cld(std::vector<std::string> &operands);
    void std(std::vector<std::string> &operands);
    void cli(std::vector<std::string> &operands);
    void sti(std::vector<std::string> &operands);
    void hlt(std::vector<std::string> &operands);
    void wait(std::vector<std::string> &operands);
    void esc(std::vector<std::string> &operands);
    void lock(std::vector<std::string> &operands);
    void nop(std::vector<std::string> &operands);
    void in_op(std::vector<std::string> &operands);
    void out(std::vector<std::string> &operands);
    void rep(std::vector<std::string> &operands);
    void repe(std::vector<std::string> &operands);
    void repne(std::vector<std::string> &operands);
    void repnz(std::vector<std::string> &operands);
    void repz(std::vector<std::string> &operands);
    void xlat(std::vector<std::string> &operands);
    void xlatb(std::vector<std::string> &operands);

public:
    explicit Emulator8086(size_t memSize = 1024 * 1024);
    void executeInstruction(const std::string &instruction);
    void displayRegisters();
    void displayStack();
    void displayMemory(uint16_t address, uint16_t count);
    void displayHelp();
};

#endif