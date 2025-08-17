#include "gui/gui_application.h"
#include <iostream>

int main_gui(int argc, char* argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv;
    
    std::cout << "Starting 8086 Emulator GUI...\n";
    
    GUIApplication app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize GUI application\n";
        return 1;
    }
    
    app.run();
    
    std::cout << "GUI application exited cleanly\n";
    return 0;
}
