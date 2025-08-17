#include <cassert>
#include <iostream>

#include "gui/gui_application.h"

int test_gui_integration() {
    std::cout << "Testing GUI Integration (Week 1)...\n";
    {
        GUIApplication app;
        std::cout << "✅ GUIApplication instantiation: OK\n";
    }
    {
        GUIApplication app;
        assert(!app.isRunning());
        std::cout << "✅ Initial running state: OK\n";
    }
    {
        for (int i = 0; i < 3; i++) {
            GUIApplication app;
        }
        std::cout << "✅ Multiple instantiation cycles: OK\n";
    }
    {
        GUIApplication app;
        app.shutdown();
        std::cout << "✅ Safe shutdown: OK\n";
    }
    std::cout << "All GUI integration tests passed!\n";
    return 0;
}
