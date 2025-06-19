#ifndef STRING_H
#define STRING_H

#include <vector>
#include <string>
#include "../registers.h"
#include "../memory_components.h"

class Emulator8086; // Forward declaration

class StringInstructions
{
private:
    Emulator8086* emulator;

public:
    StringInstructions(Emulator8086* emu);
    
    // String instructions
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
    void rep(const std::vector<std::string> &operands);
    void repe(const std::vector<std::string> &operands);
    void repne(const std::vector<std::string> &operands);
    void repnz(const std::vector<std::string> &operands);
    void repz(const std::vector<std::string> &operands);
    void xlat(const std::vector<std::string> &operands);
    void xlatb(const std::vector<std::string> &operands);
};

#endif
