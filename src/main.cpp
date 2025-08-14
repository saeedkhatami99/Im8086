#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "emulator8086.h"
#include "tui.h"

int main(int argc, char **argv)
{
    Emulator8086 emu;
    if (argc >= 3 && std::string(argv[1]) == "--tui")
    {
        std::ifstream fin(argv[2]);
        if (!fin)
        {
            std::cerr << "Failed to open program file: " << argv[2] << "\n";
            return 1;
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(fin, line))
            lines.push_back(line);
        emu.loadProgram(lines);
        EmulatorTUI tui(&emu);
        tui.run();
        return 0;
    }

    std::string input;

    std::cout << "8086 Emulator\n";
    std::cout << "Commands:\n";
    std::cout << "  '?' - display help\n";
    std::cout << "  'reg' - display registers\n";
    std::cout << "  'stack' - display stack\n";
    std::cout << "  'mem ADDR COUNT' - display memory (e.g., 'mem 100 128')\n";
    std::cout << "  '3xit' - quit emulator\n";
    std::cout << "Enter assembly instructions:\n";

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == ":3xit")
            break;
        if (input == "?")
        {
            emu.displayHelp();
            continue;
        }
        if (input == "reg")
        {
            emu.displayRegisters();
            continue;
        }
        if (input == "stack")
        {
            emu.displayStack();
            continue;
        }
        if (input.substr(0, 4) == "mem ")
        {
            std::istringstream iss(input.substr(4));
            uint16_t addr, count;
            iss >> std::hex >> addr >> count;
            emu.displayMemory(addr, count);
            continue;
        }
        if (input.empty())
            continue;

        try
        {
            emu.executeInstruction(input);
        }
        catch (const std::exception &e)
        {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}