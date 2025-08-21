#define SDL_MAIN_HANDLED

#ifdef _WIN32
#include "opengl_modern.h"
#else
#include "platform_opengl.h"
#endif

#include "gui/gui_application.h"
#include "test_framework.h"

TEST_CASE(GUIApplicationConstruction) {
    GUIApplication app;

    REQUIRE(!app.isRunning());
}

TEST_CASE(GUIApplicationMultipleInstantiation) {
    for (int i = 0; i < 3; i++) {
        GUIApplication app;
        REQUIRE(!app.isRunning());
    }
}

TEST_CASE(GUIApplicationSafeShutdown) {
    GUIApplication app;

    app.shutdown();
    REQUIRE(!app.isRunning());
}

TEST_CASE(GUIApplicationLoadAssemblyFile) {
    GUIApplication app;

    bool result = app.loadAssemblyFile("nonexistent_file.asm");
    REQUIRE(!result);
}

int main() {
    return TestFramework::instance().runAll();
}
