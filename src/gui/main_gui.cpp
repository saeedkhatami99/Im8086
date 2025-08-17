#include <iostream>
#include <string>

#include "gui/gui_application.h"
#include "version.h"

int main_gui(int argc, char* argv[]) {
    std::cout << "Starting Im8086 Emulator GUI v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << " (" << BUILD_PLATFORM << ")...\n";
    
    std::string window_title = "Im8086 Emulator v" + std::to_string(PROJECT_VERSION_MAJOR) + "." + 
                              std::to_string(PROJECT_VERSION_MINOR) + "." + std::to_string(PROJECT_VERSION_PATCH) + 
                              " - Intel 8086 Microprocessor Emulator";
    
    GUIApplication app;
    if (!app.initialize(1280, 720, window_title)) {
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
