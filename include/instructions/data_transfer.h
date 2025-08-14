#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086;

class DataTransferInstructions
{
private:
    Emulator8086 *emulator;

public:
    DataTransferInstructions(Emulator8086 *emu);

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
};

#endif
