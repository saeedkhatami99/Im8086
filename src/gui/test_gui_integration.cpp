#include <cassert>
#include <iostream>
#include "gui/gui_application.h"

// Simple integration test to verify GUI classes can be instantiated
// and basic functionality works without requiring graphics context

int test_gui_integration() {
    std::cout << "Testing GUI Integration (Week 1)...\n";
    
    // Test 1: GUIApplication can be instantiated
    {
        GUIApplication app;
        std::cout << "✅ GUIApplication instantiation: OK\n";
    }
    
    // Test 2: Application starts in non-running state
    {
        GUIApplication app;
        assert(!app.isRunning());
        std::cout << "✅ Initial running state: OK\n";
    }
    
    // Test 3: Multiple instantiation/destruction cycles work
    {
        for (int i = 0; i < 3; i++) {
            GUIApplication app;
            // App destructor should clean up properly
        }
        std::cout << "✅ Multiple instantiation cycles: OK\n";
    }
    
    // Test 4: Shutdown on uninitialized app is safe
    {
        GUIApplication app;
        app.shutdown(); // Should be safe to call
        std::cout << "✅ Safe shutdown: OK\n";
    }
    
    std::cout << "All GUI integration tests passed!\n";
    return 0;
}

// This can be called from main() for testing
// Note: Full GUI testing requires graphics context
