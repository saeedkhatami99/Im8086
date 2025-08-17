#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

struct ImGuiContext;
struct ImGuiInputTextCallbackData;
typedef union SDL_Event SDL_Event;

class Emulator8086;

class GUIApplication {
  public:
    GUIApplication();
    ~GUIApplication();

    bool initialize(int width = 1280, int height = 720, const std::string& title = "Im8086 Emulator");
    bool loadAssemblyFile(const std::string& filePath);
    void run();
    void shutdown();

    bool isRunning() const {
        return running;
    }

  private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    ImGuiContext* imguiContext = nullptr;

    std::unique_ptr<Emulator8086> emulator;
    std::string loadedFilePath;
    std::vector<std::string> assemblyLines;

    bool running = false;
    bool initialized = false;

    bool showRegistersWindow = true;
    bool showMemoryWindow = true;
    bool showDemoWindow = true;
    bool showAssemblyEditor = true;
    bool showStackWindow = true;
    bool showFileDialog = false;

    int memoryViewStart = 0;
    int memoryViewSize = 256;
    int stackViewSize = 32;

    bool assemblyEditorModified = false;
    std::string assemblyEditorBuffer;
    static constexpr size_t EDITOR_BUFFER_SIZE = 65536;
    char assemblyEditorCharBuffer[EDITOR_BUFFER_SIZE];
    int currentLine = 0;

    bool initSDL(int width, int height, const std::string& title);
    bool initOpenGL();
    bool initImGui();

    void handleEvents();
    void update();
    void render();

    void handleKeyDown(const SDL_Event& event);
    void handleWindowEvent(const SDL_Event& event);

    void renderImGui();
    void renderMainMenuBar();
    void renderDemoWindow();
    void renderAssemblyEditor();
    void renderEmulatorStatus();
    void renderRegistersWindow();
    void renderMemoryWindow();
    void renderStackWindow();
    void renderFileDialog();

    std::string openFileDialog(const std::string& title = "Open Assembly File",
                               const std::string& filters = "*.asm;*.s;*.as");
    bool isFileDialogAvailable();

    void syncEditorBuffer();
    void updateAssemblyLinesFromBuffer();
    bool saveAssemblyFile(const std::string& filePath);
    void assembleAndLoad();
    int getCurrentLineNumber();
    static int textEditCallback(ImGuiInputTextCallbackData* data);

    void cleanupImGui();
    void cleanupOpenGL();
    void cleanupSDL();
};

#endif
