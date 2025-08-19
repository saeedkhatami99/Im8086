#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "emulator8086.h"
#include "ide_tui.h"
#include "tui.h"
#include "version.h"
#ifdef WITH_GUI
int main_gui(int argc, char* argv[]);
#endif

int main(int argc, char** argv) {
    Emulator8086 emu;

#ifdef WITH_GUI
    if (argc >= 2 && std::string(argv[1]) == "--gui") {
        return main_gui(argc, argv);
    }
#endif

#ifdef WITH_TUI
    if (argc >= 2 && std::string(argv[1]) == "--ide") {
        EmulatorIDETUI ide(&emu);
        ide.run();
        return 0;
    }
#endif

#ifdef WITH_TUI
    if (argc >= 3 && std::string(argv[1]) == "--tui") {
        std::ifstream fin(argv[2]);
        if (!fin) {
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
#endif

    if (argc > 1) {
        std::cout << "Im8086 Emulator v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR
                  << "." << PROJECT_VERSION_PATCH << " (" << BUILD_PLATFORM << ")\n";
        // std::cout << "Intel 8086 Microprocessor Emulator\n\n";
        std::cout << "\n";
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << "                    - Interactive command line mode\n";
#ifdef WITH_GUI
        std::cout << "  " << argv[0] << " --gui             - Modern GUI mode with SDL2/ImGui\n";
        std::cout << "  " << argv[0] << " --gui <file>      - GUI mode with assembly file loaded\n";
#endif
#ifdef WITH_TUI
        std::cout << "  " << argv[0]
                  << " --ide             - IDE mode with integrated editor and debugger\n";
        std::cout << "  " << argv[0]
                  << " --tui <file>      - TUI debugger mode with assembly file\n";
#endif

#ifdef WITH_GUI
        std::cout << "\nGUI Mode Features:\n";
        std::cout << "  - Cross-platform graphical interface\n";
        std::cout << "  - Integrated assembly editor\n";
        std::cout << "  - Visual debugging and memory inspection\n";
        std::cout << "  - Real-time register monitoring\n";
#endif

#ifdef WITH_TUI
        std::cout << "\nIDE Mode Features:\n";
        std::cout << "  - Integrated assembly code editor\n";
        std::cout << "  - Real-time compilation and debugging\n";
        std::cout << "  - Step-by-step execution\n";
        std::cout << "  - Breakpoint support\n";
        std::cout << "  - Register and memory inspection\n";
#endif
        std::cout << "  - Save/Load programs\n";
        return 0;
    }

    std::string input;

    std::cout << "Im8086 Emulator v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << "."
              << PROJECT_VERSION_PATCH << " (" << BUILD_PLATFORM << ")\n";
    // std::cout << "Intel 8086 Microprocessor Emulator\n\n";
    std::cout << "\n";
    std::cout << "Commands:\n";
    std::cout << "  '?' - display help\n";
    std::cout << "  'reg' - display registers\n";
    std::cout << "  'stack' - display stack\n";
    std::cout << "  'mem ADDR COUNT' - display memory (e.g., 'mem 100 128')\n";
    std::cout << "  '3xit' - quit emulator\n";
    std::cout << "Enter assembly instructions:\n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == ":3xit" || input == "exit" || input == "quit" || input == "3xit")
            break;
        if (input == "?") {
            emu.displayHelp();
            continue;
        }
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
        if (input.empty())
            continue;

        try {
            emu.executeInstruction(input);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}