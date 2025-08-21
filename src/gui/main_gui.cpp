#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#define NOGDI
#endif

#include <iostream>
#include <string>

#include "gui/gui_application.h"
#include "gui/splash_window.h"
#include "version.h"

int main_gui(int argc, char* argv[]) {
    std::cout << "Starting Im8086 Emulator GUI v" << PROJECT_VERSION_MAJOR << "."
              << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << " (" << BUILD_PLATFORM
              << ")...\n";

    std::string window_title = "Im8086 Emulator v" + std::to_string(PROJECT_VERSION_MAJOR) + "." +
                               std::to_string(PROJECT_VERSION_MINOR) + "." +
                               std::to_string(PROJECT_VERSION_PATCH);
    //    " - Intel 8086 Microprocessor Emulator";

    SplashWindow splash;
    bool splashCreated = false;
    std::cout << "Attempting to create splash window..." << std::endl;
    if (splash.create("LogoWithBG.png", 300)) {
        std::cout << "Splash window created successfully, showing with fade..." << std::endl;
        
        splash.showWithFade(1.5f, 2.0f, 1.5f);
        
        splashCreated = true;
        std::cout << "Splash animation completed" << std::endl;
    } else {
        std::cout << "Failed to create splash window" << std::endl;
    }

    GUIApplication app;
    if (!app.initialize(1280, 720, window_title)) {
        std::cerr << "Failed to initialize GUI application\n";
        if (splashCreated) splash.destroy();
        return 1;
    }
    
    if (splashCreated) {
        std::cout << "Splash animation completed, window already hidden" << std::endl;
    }
    if (argc >= 3) {
        std::string filename = argv[2];
        std::cout << "Loading assembly file: " << filename << "\n";
        if (!app.loadAssemblyFile(filename)) {
            std::cerr << "Failed to load assembly file: " << filename << "\n";
        }
    }
    app.run();
    
    if (splashCreated) {
        splash.destroy();
    }
    
    std::cout << "GUI application exited cleanly\n";
    return 0;
}
