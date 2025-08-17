#include <iostream>
#include <string>

#include "gui/gui_application.h"

int main_gui(int argc, char* argv[]) {
    std::cout << "Starting 8086 Emulator GUI...\n";
    GUIApplication app;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize GUI application\n";
        return 1;
    }
    if (argc >= 3) {
        std::string filename = argv[2];
        std::cout << "Loading assembly file: " << filename << "\n";
        if (!app.loadAssemblyFile(filename)) {
            std::cerr << "Failed to load assembly file: " << filename << "\n";
        }
    }
    app.run();
    std::cout << "GUI application exited cleanly\n";
    return 0;
}
