#ifndef LOGICAL_H
#define LOGICAL_H

#include <string>
#include <vector>

#include "../memory_components.h"
#include "../registers.h"

class Emulator8086;

class LogicalInstructions {
  private:
    Emulator8086* emulator;

  public:
    LogicalInstructions(Emulator8086* emu);

    void and_op(const std::vector<std::string>& operands);
    void or_op(const std::vector<std::string>& operands);
    void xor_op(const std::vector<std::string>& operands);
    void not_op(const std::vector<std::string>& operands);
    void test(const std::vector<std::string>& operands);
    void cmp(const std::vector<std::string>& operands);
};

#endif
