#include "test_framework.h"
#include "gui/gui_application.h"

TEST_CASE(GUIApplicationConstruction) {
    GUIApplication app;
    
    // Test initial state
    REQUIRE(!app.isRunning());
}

TEST_CASE(GUIApplicationMultipleInstantiation) {
    // Test that multiple instances can be created and destroyed safely
    for (int i = 0; i < 3; i++) {
        GUIApplication app;
        REQUIRE(!app.isRunning());
    }
}

TEST_CASE(GUIApplicationSafeShutdown) {
    GUIApplication app;
    
    // Should be safe to call shutdown on uninitialized app
    app.shutdown();
    REQUIRE(!app.isRunning());
}

TEST_CASE(GUIApplicationLoadAssemblyFile) {
    GUIApplication app;
    
    // Note: This test requires GUI initialization which requires graphics context
    // For now, just test that the method exists and doesn't crash on invalid files
    bool result = app.loadAssemblyFile("nonexistent_file.asm");
    REQUIRE(!result);  // Should fail for non-existent file
}

int main() {
    return TestFramework::instance().runAll();
}
