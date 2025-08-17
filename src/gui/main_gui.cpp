#include "gui/gui_application.h"
#include <iostream>
#include <string>

int main_gui(int argc, char* argv[]) {
    std::cout << "Starting 8086 Emulator GUI...\n";
    
    GUIApplication app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize GUI application\n";
        return 1;
    }
    
    // Check for assembly file argument
    // Arguments: ./8086emu --gui [filename]
    if (argc >= 3) {
        std::string filename = argv[2];
        std::cout << "Loading assembly file: " << filename << "\n";
        
        if (!app.loadAssemblyFile(filename)) {
            std::cerr << "Failed to load assembly file: " << filename << "\n";
            // Continue anyway - user can still use the GUI
        }
    }
    
    app.run();
    
    std::cout << "GUI application exited cleanly\n";
    return 0;
}
