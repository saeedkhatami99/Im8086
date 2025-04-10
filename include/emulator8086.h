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
    // void updateFlags(uint16_t result, bool checkCarry = true);
    void updateFlags(uint32_t result, bool isByte, bool checkCarry);
    MemoryOperand parseMemoryOperand(const std::string &operand);
    uint16_t calculateEffectiveAddress(const MemoryOperand &memOp);
    uint16_t readMemoryWord(uint16_t address);
    void writeMemoryWord(uint16_t address, uint16_t value);
    uint8_t readMemoryByte(uint16_t address);
    void writeMemoryByte(uint16_t address, uint8_t value);

    void mov(const std::vector<std::string> &operands);
    void push(const std::vector<std::string> &operands);
    void pop(const std::vector<std::string> &operands);
    void xchg(const std::vector<std::string> &operands);
    void lea(const std::vector<std::string> &operands);
    void lds(const std::vector<std::string> &operands);
    void les(const std::vector<std::string> &operands);
    void lahf(const std::vector<std::string> &operands);
    void sahf(const std::vector<std::string> &operands);
    void pushf(const std::vector<std::string> &operands);
    void popf(const std::vector<std::string> &operands);
    void pusha(const std::vector<std::string> &operands);
    void popa(const std::vector<std::string> &operands);
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
    void and_op(const std::vector<std::string> &operands);
    void or_op(const std::vector<std::string> &operands);
    void xor_op(const std::vector<std::string> &operands);
    void not_op(const std::vector<std::string> &operands);
    void test(const std::vector<std::string> &operands);
    void cmp(const std::vector<std::string> &operands);
    void movsb(const std::vector<std::string> &operands);
    void movsw(const std::vector<std::string> &operands);
    void cmpsb(const std::vector<std::string> &operands);
    void cmpsw(const std::vector<std::string> &operands);
    void scasb(const std::vector<std::string> &operands);
    void scasw(const std::vector<std::string> &operands);
    void lodsb(const std::vector<std::string> &operands);
    void lodsw(const std::vector<std::string> &operands);
    void stosb(const std::vector<std::string> &operands);
    void stosw(const std::vector<std::string> &operands);
    void call(const std::vector<std::string> &operands);
    void jmp(const std::vector<std::string> &operands);
    void ret(const std::vector<std::string> &operands);
    void retf(const std::vector<std::string> &operands);
    void je(const std::vector<std::string> &operands);
    void jl(const std::vector<std::string> &operands);
    void jle(const std::vector<std::string> &operands);
    void jb(const std::vector<std::string> &operands);
    void jbe(const std::vector<std::string> &operands);
    void jp(const std::vector<std::string> &operands);
    void jo(const std::vector<std::string> &operands);
    void js(const std::vector<std::string> &operands);
    void jne(const std::vector<std::string> &operands);
    void jnl(const std::vector<std::string> &operands);
    void jg(const std::vector<std::string> &operands);
    void jnb(const std::vector<std::string> &operands);
    void ja(const std::vector<std::string> &operands);
    void jnp(const std::vector<std::string> &operands);
    void jno(const std::vector<std::string> &operands);
    void jns(const std::vector<std::string> &operands);
    void loop(const std::vector<std::string> &operands);
    void loopz(const std::vector<std::string> &operands);
    void loopnz(const std::vector<std::string> &operands);
    void jcxz(const std::vector<std::string> &operands);
    void int_op(const std::vector<std::string> &operands);
    void into(const std::vector<std::string> &operands);
    void iret(const std::vector<std::string> &operands);
    void rcl(const std::vector<std::string> &operands);
    void rcr(const std::vector<std::string> &operands);
    void rol(const std::vector<std::string> &operands);
    void ror(const std::vector<std::string> &operands);
    void sal(const std::vector<std::string> &operands);
    void sar(const std::vector<std::string> &operands);
    void shl(const std::vector<std::string> &operands);
    void shr(const std::vector<std::string> &operands);
    void clc(const std::vector<std::string> &operands);
    void cmc(const std::vector<std::string> &operands);
    void stc(const std::vector<std::string> &operands);
    void cld(const std::vector<std::string> &operands);
    void std(const std::vector<std::string> &operands);
    void cli(const std::vector<std::string> &operands);
    void sti(const std::vector<std::string> &operands);
    void hlt(const std::vector<std::string> &operands);
    void wait(const std::vector<std::string> &operands);
    void esc(const std::vector<std::string> &operands);
    void lock(const std::vector<std::string> &operands);
    void nop(const std::vector<std::string> &operands);
    void in_op(const std::vector<std::string> &operands);
    void out(const std::vector<std::string> &operands);
    void rep(const std::vector<std::string> &operands);
    void repe(const std::vector<std::string> &operands);
    void repne(const std::vector<std::string> &operands);
    void repnz(const std::vector<std::string> &operands);
    void repz(const std::vector<std::string> &operands);
    void xlat(const std::vector<std::string> &operands);
    void xlatb(const std::vector<std::string> &operands);

public:
    explicit Emulator8086(size_t memSize = 1024 * 1024);
    void executeInstruction(const std::string &instruction);
    void displayRegisters();
    void displayStack();
    void displayMemory(uint16_t address, uint16_t count);
    void displayHelp();
};

#endif