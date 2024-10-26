#include <iostream>
#include <string>
#include <sstream>
#include "emulator8086.h"

int main() {
    Emulator8086 emu;
    std::string input;

    std::cout << "8086 Emulator\n";
    std::cout << "Commands:\n";
    std::cout << "  'reg' - display registers\n";
    std::cout << "  'stack' - display stack\n";
    std::cout << "  'mem ADDR COUNT' - display memory (e.g., 'mem 100 128')\n";
    std::cout << "  'exit' - quit emulator\n";
    std::cout << "Enter assembly instructions:\n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") break;
        if (input == "reg") {
            emu.displayRegisters();
            continue;
        }
        if (input == "stack") {
            emu.displayStack();
            continue;
        }
        if (input.substr(0, 4) == "mem ") {
            std::istringstream iss(input.substr(4));
            uint16_t addr, count;
            iss >> std::hex >> addr >> count;
            emu.displayMemory(addr, count);
            continue;
        }
        if (input.empty()) continue;

        try {
            emu.executeInstruction(input);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}