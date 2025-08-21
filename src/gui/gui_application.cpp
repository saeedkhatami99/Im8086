#ifdef USE_GLAD
#include "opengl_modern.h"
#else
#include "platform_opengl.h"
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "gui/gui_application.h"
#include "image_loader.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <imgui_internal.h>

#include "emulator8086.h"
#include "version.h"

namespace {

bool setEnvironmentVariable(const char* name, const char* value) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name, value) != 0;
#else
    return setenv(name, value, 1) == 0;
#endif
}

std::string getEnvironmentVariable(const char* name) {
#ifdef _WIN32
    char buffer[1024];
    DWORD result = GetEnvironmentVariableA(name, buffer, sizeof(buffer));
    if (result > 0 && result < sizeof(buffer)) {
        return std::string(buffer);
    }
    return "";
#else
    const char* value = std::getenv(name);
    return value ? std::string(value) : "";
#endif
}

std::string executeCommand(const std::string& command) {
#ifdef _WIN32

    return "";
#else
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }

    char buffer[1024];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
#endif
}

bool commandExists(const std::string& command) {
#ifdef _WIN32
    return false;
#else
    FILE* pipe = popen((command + " 2>/dev/null").c_str(), "r");
    if (!pipe) {
        return false;
    }

    char buffer[128];
    bool found = fgets(buffer, sizeof(buffer), pipe) != nullptr;
    pclose(pipe);
    return found;
#endif
}
}  // namespace

GUIApplication::GUIApplication() {
    assemblyEditorCharBuffer[0] = '\0';
}
GUIApplication::~GUIApplication() {
    shutdown();
}
bool GUIApplication::initialize(int width, int height, const std::string& title) {
    if (initialized) {
        return true;
    }

    const std::string software_env = getEnvironmentVariable("LIBGL_ALWAYS_SOFTWARE");
    bool force_software = (software_env == "1");

    if (force_software) {
        std::cout << "Software rendering requested via LIBGL_ALWAYS_SOFTWARE" << std::endl;
    }

    try {
        if (!initSDL(width, height, title)) {
            throw std::runtime_error("Failed to initialize SDL2");
        }

        if (!initOpenGL()) {
            if (!force_software) {
                std::cout << "Hardware OpenGL failed, trying software rendering..." << std::endl;
                std::cout
                    << "Note: Software rendering will be slower but should work on any system."
                    << std::endl;

                if (glContext) {
                    SDL_GL_DeleteContext(glContext);
                    glContext = nullptr;
                }
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                }
                SDL_Quit();

                setEnvironmentVariable("LIBGL_ALWAYS_SOFTWARE", "1");

                if (!initSDL(width, height, title)) {
                    throw std::runtime_error("Failed to initialize SDL2 with software rendering");
                }

                if (!initOpenGL()) {
                    throw std::runtime_error(
                        "Failed to initialize OpenGL even with software rendering");
                }

                std::cout << "Successfully initialized with software rendering" << std::endl;
            } else {
                throw std::runtime_error("Failed to initialize OpenGL with software rendering");
            }
        }

        if (!initImGui()) {
            throw std::runtime_error("Failed to initialize ImGui");
        }

        emulator = std::make_unique<Emulator8086>();

        running = true;
        initialized = true;

        std::cout << "GUI Application initialized successfully\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        shutdown();
        return false;
    }
}

bool GUIApplication::loadAssemblyFile(const std::string& filePath) {
    if (!initialized) {
        std::cerr << "Application not initialized. Call initialize() first.\n";
        return false;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    assemblyLines.clear();

    std::string line;
    while (std::getline(file, line)) {
        assemblyLines.push_back(line);
    }

    file.close();

    if (assemblyLines.empty()) {
        std::cerr << "Warning: File is empty: " << filePath << std::endl;
        return false;
    }

    loadedFilePath = filePath;

    if (emulator) {
        try {
            emulator->reset();
            emulator->loadProgram(assemblyLines);
            std::cout << "Successfully loaded " << assemblyLines.size() << " lines from "
                      << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load program into emulator: " << e.what() << std::endl;
            return false;
        }
    }

    std::cerr << "Emulator not initialized" << std::endl;
    return false;
}

void GUIApplication::run() {
    if (!initialized) {
        std::cerr << "Application not initialized. Call initialize() first.\n";
        return;
    }

    std::cout << "Starting main loop...\n";

    while (running) {
        handleEvents();
        update();
        render();
    }

    std::cout << "Main loop ended.\n";
}

void GUIApplication::shutdown() {
    if (!initialized) {
        return;
    }

    running = false;

    ImageLoader::unloadImage(logoImage);

    emulator.reset();
    cleanupImGui();
    cleanupOpenGL();
    cleanupSDL();

    initialized = false;
    std::cout << "GUI Application shutdown complete\n";
}

bool GUIApplication::initSDL(int width, int height, const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL2 initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    struct OpenGLConfig {
        int major, minor;
        int profile;
        int flags;
        const char* description;
    };

    OpenGLConfig configs[] = {

        {3,
         3,
         SDL_GL_CONTEXT_PROFILE_CORE,
         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG,
         "OpenGL 3.3 Core"},

        {3, 0, SDL_GL_CONTEXT_PROFILE_CORE, 0, "OpenGL 3.0 Core"},

        {2, 1, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 0, "OpenGL 2.1 Compatibility"},

        {0, 0, 0, 0, "OpenGL Legacy"}};

    window = nullptr;

    for (auto& config : configs) {
        std::cout << "Trying " << config.description << "..." << std::endl;

        if (config.major > 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.minor);
        }

        if (config.profile > 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, config.profile);
        }

        if (config.flags > 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, config.flags);
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        window =
            SDL_CreateWindow(title.c_str(),
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             width,
                             height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

        if (window) {
            std::cout << "Successfully created window with " << config.description << std::endl;
            
            SDL_Surface* iconSurface = ImageLoader::loadImageForIcon("Icon.png");
            if (iconSurface) {
                SDL_SetWindowIcon(window, iconSurface);
                SDL_FreeSurface(iconSurface);
                std::cout << "Window icon set successfully" << std::endl;
            } else {
                std::cout << "Warning: Could not load window icon (Icon.png)" << std::endl;
            }
            
            break;
        } else {
            std::cout << "Failed to create window with " << config.description << ": "
                      << SDL_GetError() << std::endl;
        }
    }

    if (!window) {
        std::cerr << "Failed to create SDL window with any OpenGL configuration" << std::endl;
        return false;
    }

    return true;
}

bool GUIApplication::initOpenGL() {
    struct OpenGLContextConfig {
        int major, minor;
        const char* description;
    };

    OpenGLContextConfig contexts[] = {
        {3, 3, "OpenGL 3.3"}, {3, 0, "OpenGL 3.0"}, {2, 1, "OpenGL 2.1"}, {0, 0, "OpenGL Legacy"}};

    for (auto& config : contexts) {
        std::cout << "Trying to create " << config.description << " context..." << std::endl;

        if (config.major > 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.minor);
        }

        glContext = SDL_GL_CreateContext(window);
        if (glContext) {
            std::cout << "Successfully created " << config.description << " context" << std::endl;
            break;
        } else {
            std::cout << "Failed to create " << config.description << " context: " << SDL_GetError()
                      << std::endl;
        }
    }

    if (!glContext) {
        std::cerr << "Failed to create any OpenGL context" << std::endl;

        std::cout << "OpenGL debug information:" << std::endl;
        int value;
        if (SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value) == 0)
            std::cout << "  SDL_GL_RED_SIZE: " << value << std::endl;
        if (SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value) == 0)
            std::cout << "  SDL_GL_GREEN_SIZE: " << value << std::endl;
        if (SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value) == 0)
            std::cout << "  SDL_GL_BLUE_SIZE: " << value << std::endl;
        if (SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value) == 0)
            std::cout << "  SDL_GL_DEPTH_SIZE: " << value << std::endl;

        return false;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        std::cerr << "Failed to make OpenGL context current: " << SDL_GetError() << std::endl;
        return false;
    }

    if (SDL_GL_SetSwapInterval(-1) != 0) {
        if (SDL_GL_SetSwapInterval(1) != 0) {
            std::cout << "Warning: VSync not supported" << std::endl;
        }
    }

    int displayW, displayH;
    SDL_GetWindowSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);

    std::cout << "OpenGL context created successfully" << std::endl;

    return true;
}

bool GUIApplication::initImGui() {
    IMGUI_CHECKVERSION();
    imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext)) {
        std::cerr << "Failed to initialize ImGui SDL2 backend\n";
        return false;
    }

    const char* glsl_version = nullptr;

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    std::cout << "Detected OpenGL " << major << "." << minor << std::endl;

    if (major >= 4 || (major == 3 && minor >= 3)) {
        glsl_version = "#version 330";
        std::cout << "Using GLSL 330 (OpenGL 3.3+)" << std::endl;
    } else if (major == 3 && minor >= 2) {
        glsl_version = "#version 150";
        std::cout << "Using GLSL 150 (OpenGL 3.2)" << std::endl;
    } else if (major == 3 && minor >= 0) {
        glsl_version = "#version 130";
        std::cout << "Using GLSL 130 (OpenGL 3.0)" << std::endl;
    } else {
        glsl_version = "#version 120";
        std::cout << "Using GLSL 120 (OpenGL 2.1)" << std::endl;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend with " << glsl_version
                  << std::endl;

        if (std::strcmp(glsl_version, "#version 120") != 0) {
            std::cout << "Trying fallback to GLSL 120..." << std::endl;
            if (!ImGui_ImplOpenGL3_Init("#version 120")) {
                std::cerr << "Failed to initialize ImGui OpenGL3 backend with fallback\n";
                return false;
            }
        } else {
            return false;
        }
    }

    std::cout << "ImGui initialized successfully with " << glsl_version << std::endl;
    return true;
}

void GUIApplication::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                handleKeyDown(event);
                break;

            case SDL_WINDOWEVENT:
                handleWindowEvent(event);
                break;
        }
    }
}

void GUIApplication::handleKeyDown(const SDL_Event& event) {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    bool ctrl = keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL];

    switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            running = false;
            break;

        case SDLK_F2:
            showRegistersWindow = !showRegistersWindow;
            break;

        case SDLK_F3:
            showMemoryWindow = !showMemoryWindow;
            break;

        case SDLK_F4:
            showAssemblyEditor = !showAssemblyEditor;
            break;

        case SDLK_F5:
            showStackWindow = !showStackWindow;
            break;

        case SDLK_F7:
            if (emulator) {
                try {
                    bool continueExecution = emulator->step();
                    if (!continueExecution) {
                        std::cout << "Program execution completed\n";
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Step execution error: " << e.what() << std::endl;
                }
            }
            break;

        case SDLK_F11:

            break;

        case SDLK_r:
            if (ctrl && emulator) {
                emulator->reset();
                if (!assemblyLines.empty()) {
                    emulator->loadProgram(assemblyLines);
                }
                std::cout << "Emulator reset\n";
            }
            break;

        case SDLK_l:
            if (ctrl && emulator && !assemblyLines.empty()) {
                try {
                    emulator->reset();
                    emulator->loadProgram(assemblyLines);
                    std::cout << "Program loaded into emulator\n";
                } catch (const std::exception& e) {
                    std::cerr << "Load program error: " << e.what() << std::endl;
                }
            }
            break;

        case SDLK_RETURN:
            if (ctrl) {
                assembleAndLoad();
            }
            break;
    }
}

void GUIApplication::handleWindowEvent(const SDL_Event& event) {
    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        int width = event.window.data1;
        int height = event.window.data2;
        glViewport(0, 0, width, height);
    }
}

void GUIApplication::update() {}

void GUIApplication::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    renderImGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

void GUIApplication::renderImGui() {
    if (showSplashScreen) {
        renderSplashScreen();
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    renderMainMenuBar();

    ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

        ImGuiID rightId =
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.25f, nullptr, &dockspaceId);
        ImGuiID bottomId =
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.4f, nullptr, &dockspaceId);
        ImGuiID leftTopId =
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.6f, nullptr, &dockspaceId);

        ImGui::DockBuilderDockWindow("Registers", rightId);
        ImGui::DockBuilderDockWindow("Memory Viewer", bottomId);
        ImGui::DockBuilderDockWindow("Assembly Editor", leftTopId);
        ImGui::DockBuilderDockWindow("Stack Viewer", dockspaceId);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    ImGui::End();

    renderAssemblyEditor();
    renderRegistersWindow();
    renderMemoryWindow();
    renderStackWindow();
    renderFileDialog();
}

void GUIApplication::renderSplashScreen() {
    if (!logoImage.loaded) {
        logoImage = ImageLoader::loadImage("LogoWithBG.png");
    }
    
    if (!logoImage.loaded) {
        showSplashScreen = false;
        return;
    }
    
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    float splashHeight = 300.0f;
    float aspectRatio = (float)logoImage.width / (float)logoImage.height;
    float splashWidth = splashHeight * aspectRatio;
    
    float padding = 10.0f;
    float windowWidth = splashWidth + (padding * 2);
    float windowHeight = splashHeight + (padding * 2);
    
    ImVec2 windowPos(
        viewport->Pos.x + (viewport->Size.x - windowWidth) * 0.5f,
        viewport->Pos.y + (viewport->Size.y - windowHeight) * 0.5f
    );
    
    ImVec2 splashPos(
        windowPos.x + padding,
        windowPos.y + padding
    );
    
    static float splashTimer = 0.0f;
    splashTimer += ImGui::GetIO().DeltaTime;
    
    float fadeInDuration = 0.5f;
    float holdDuration = 2.5f;
    float fadeOutDuration = 0.5f;
    float totalDuration = fadeInDuration + holdDuration + fadeOutDuration;
    
    float alpha = 1.0f;
    if (splashTimer < fadeInDuration) {
        alpha = splashTimer / fadeInDuration;
    } else if (splashTimer > fadeInDuration + holdDuration) {
        float fadeProgress = (splashTimer - fadeInDuration - holdDuration) / fadeOutDuration;
        alpha = 1.0f - fadeProgress;
    }

    alpha = std::max(0.0f, std::min(1.0f, alpha));
    
    if (splashTimer > totalDuration) {
        showSplashScreen = false;
        return;
    }

    ImU32 backgroundColor = IM_COL32(45, 45, 48, (int)(alpha * 240));
    ImU32 borderColor = IM_COL32(100, 100, 100, (int)(alpha * 180));
    ImVec2 shadowOffset(3.0f, 3.0f);
    ImU32 shadowColor = IM_COL32(0, 0, 0, (int)(alpha * 80));
    drawList->AddRectFilled(
        ImVec2(windowPos.x + shadowOffset.x, windowPos.y + shadowOffset.y),
        ImVec2(windowPos.x + windowWidth + shadowOffset.x, windowPos.y + windowHeight + shadowOffset.y),
        shadowColor,
        8.0f
    );
    
    drawList->AddRectFilled(
        windowPos,
        ImVec2(windowPos.x + windowWidth, windowPos.y + windowHeight),
        backgroundColor,
        8.0f
    );

    drawList->AddRect(
        windowPos,
        ImVec2(windowPos.x + windowWidth, windowPos.y + windowHeight),
        borderColor,
        8.0f,
        0,
        1.5f
    );

    ImU32 logoColor = IM_COL32(255, 255, 255, (int)(alpha * 255));
    drawList->AddImage(
        (void*)(intptr_t)logoImage.textureID,
        splashPos,
        ImVec2(splashPos.x + splashWidth, splashPos.y + splashHeight),
        ImVec2(0, 0),
        ImVec2(1, 1),
        logoColor
    );

    if (ImGui::IsKeyPressed(ImGuiKey_Space) || 
        ImGui::IsKeyPressed(ImGuiKey_Enter) || 
        ImGui::IsKeyPressed(ImGuiKey_Escape) ||
        ImGui::IsMouseClicked(0) || 
        ImGui::IsMouseClicked(1)) {
        showSplashScreen = false;
    }
}

void GUIApplication::renderMainMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator")) {
            if (ImGui::MenuItem("Reset", "Ctrl+R")) {
                if (emulator) {
                    emulator->reset();

                    if (!assemblyLines.empty()) {
                        emulator->loadProgram(assemblyLines);
                    }
                    std::cout << "Emulator reset\n";
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Step Execute", "F7")) {
                if (emulator) {
                    try {
                        bool continueExecution = emulator->step();
                        if (!continueExecution) {
                            std::cout << "Program execution completed\n";
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Step execution error: " << e.what() << std::endl;
                    }
                }
            }
            if (ImGui::MenuItem("Load Program", "Ctrl+L")) {
                if (emulator && !assemblyLines.empty()) {
                    try {
                        emulator->reset();
                        emulator->loadProgram(assemblyLines);
                        std::cout << "Program loaded into emulator\n";
                    } catch (const std::exception& e) {
                        std::cerr << "Load program error: " << e.what() << std::endl;
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void GUIApplication::renderDemoWindow() {
    if (!showDemoWindow)
        return;

    if (ImGui::Begin("8086 Emulator - Week 1 Demo", &showDemoWindow)) {
        ImGui::Text("SDL2 + ImGui Integration Success!");
        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);

        if (emulator) {
            ImGui::Text("Emulator initialized: Yes");
            ImGui::Text("Memory size: %zu bytes", emulator->getMemory().size());
        } else {
            ImGui::Text("Emulator initialized: No");
        }

        ImGui::Separator();

        if (ImGui::Button("Test Reset Emulator")) {
            if (emulator) {
                emulator->reset();
                std::cout << "Emulator reset via GUI\n";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Close Application")) {
            running = false;
        }

        ImGui::End();
    }
}

void GUIApplication::renderAssemblyEditor() {
    if (ImGui::Begin("Assembly Editor")) {
        if (ImGui::Button("New")) {
            assemblyLines.clear();
            assemblyLines.push_back("; New 8086 Assembly Program");
            assemblyLines.push_back("");
            assemblyEditorModified = true;
            loadedFilePath = "";
            syncEditorBuffer();
        }

        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            showFileDialog = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (!loadedFilePath.empty()) {
                saveAssemblyFile(loadedFilePath);
                assemblyEditorModified = false;
            } else {
                ImGui::OpenPopup("Save As");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Assemble & Load")) {
            assembleAndLoad();
        }

        if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char saveFilepath[256] = "";
            ImGui::Text("Enter filename to save:");
            ImGui::InputText("##savefilepath", saveFilepath, sizeof(saveFilepath));

            if (ImGui::Button("Save", ImVec2(120, 0))) {
                if (strlen(saveFilepath) > 0) {
                    if (saveAssemblyFile(saveFilepath)) {
                        loadedFilePath = saveFilepath;
                        assemblyEditorModified = false;
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        if (!loadedFilePath.empty()) {
            ImGui::Text("File: %s", loadedFilePath.c_str());
            if (assemblyEditorModified) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "*modified*");
            }
        } else {
            ImGui::Text("New file (unsaved)");
            if (assemblyEditorModified) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "*modified*");
            }
        }

        ImGui::Text("Lines: %zu", assemblyLines.size());
        ImGui::Separator();

        static bool firstTime = true;
        if (firstTime && !assemblyLines.empty()) {
            syncEditorBuffer();
            firstTime = false;
        }

        ImGui::Text("Assembly Code (Ctrl+Enter to assemble):");
        ImVec2 textSize = ImVec2(-1.0f, -ImGui::GetFrameHeightWithSpacing() * 2);

        bool contentChanged = ImGui::InputTextMultiline(
            "##assemblycode",
            assemblyEditorCharBuffer,
            EDITOR_BUFFER_SIZE,
            textSize,
            ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackEdit,
            textEditCallback,
            this);

        if (contentChanged) {
            assemblyEditorModified = true;
            assemblyEditorBuffer = assemblyEditorCharBuffer;
            updateAssemblyLinesFromBuffer();
        }

        ImGui::Separator();
        ImGui::Text("Cursor: Line %d", getCurrentLineNumber());
        ImGui::SameLine();
        if (assemblyEditorModified) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Modified");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Saved");
        }

        ImGui::End();
    }
}

void GUIApplication::renderEmulatorStatus() {
    static bool show_status = true;

    if (show_status && emulator) {
        ImGui::Begin("Emulator Status", &show_status);

        ImGui::Text("8086 Emulator State");
        ImGui::Separator();

        ImGui::Text("Memory Size: %zu bytes", emulator->getMemory().size());
        ImGui::Text("Current IP: %04X", (unsigned int)emulator->getIP());

        const auto& program = emulator->getProgram();
        ImGui::Text("Program Lines: %zu", program.size());

        if (!program.empty() && emulator->getIP() < program.size()) {
            ImGui::Text("Current Instruction: %s", program[emulator->getIP()].c_str());
        }

        ImGui::Separator();

        if (ImGui::Button("Step Execute (F7)")) {
            if (emulator) {
                try {
                    bool continueExecution = emulator->step();
                    if (!continueExecution) {
                        std::cout << "Program execution completed\n";
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Step execution error: " << e.what() << std::endl;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset (Ctrl+R)")) {
            if (emulator) {
                emulator->reset();
                if (!assemblyLines.empty()) {
                    emulator->loadProgram(assemblyLines);
                }
                std::cout << "Emulator reset\n";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Load Program (Ctrl+L)")) {
            if (emulator && !assemblyLines.empty()) {
                try {
                    emulator->reset();
                    emulator->loadProgram(assemblyLines);
                    std::cout << "Program loaded into emulator\n";
                } catch (const std::exception& e) {
                    std::cerr << "Load program error: " << e.what() << std::endl;
                }
            }
        }

        ImGui::End();
    }
}

void GUIApplication::renderRegistersWindow() {
    if (!emulator)
        return;

    if (ImGui::Begin("Registers")) {
        const auto& regs = emulator->getRegisters();

        ImGui::Text("8086 CPU Registers");
        ImGui::Separator();

        if (ImGui::CollapsingHeader("General Purpose Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(3, "RegColumns");
            ImGui::Text("Register");
            ImGui::NextColumn();
            ImGui::Text("16-bit");
            ImGui::NextColumn();
            ImGui::Text("8-bit (H/L)");
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::Text("AX");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.AX.x);
            ImGui::NextColumn();
            ImGui::Text("AH=%02X  AL=%02X", regs.AX.bytes.h, regs.AX.bytes.l);
            ImGui::NextColumn();

            ImGui::Text("BX");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.BX.x);
            ImGui::NextColumn();
            ImGui::Text("BH=%02X  BL=%02X", regs.BX.bytes.h, regs.BX.bytes.l);
            ImGui::NextColumn();

            ImGui::Text("CX");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.CX.x);
            ImGui::NextColumn();
            ImGui::Text("CH=%02X  CL=%02X", regs.CX.bytes.h, regs.CX.bytes.l);
            ImGui::NextColumn();

            ImGui::Text("DX");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.DX.x);
            ImGui::NextColumn();
            ImGui::Text("DH=%02X  DL=%02X", regs.DX.bytes.h, regs.DX.bytes.l);
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Index & Pointer Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, "IndexColumns");
            ImGui::Text("Register");
            ImGui::NextColumn();
            ImGui::Text("Value");
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::Text("SI (Source Index)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.SI);
            ImGui::NextColumn();

            ImGui::Text("DI (Destination Index)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.DI);
            ImGui::NextColumn();

            ImGui::Text("BP (Base Pointer)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.BP);
            ImGui::NextColumn();

            ImGui::Text("SP (Stack Pointer)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.SP);
            ImGui::NextColumn();

            ImGui::Text("IP (Instruction Pointer)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.IP);
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Segment Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, "SegmentColumns");
            ImGui::Text("Register");
            ImGui::NextColumn();
            ImGui::Text("Value");
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::Text("CS (Code Segment)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.CS);
            ImGui::NextColumn();

            ImGui::Text("DS (Data Segment)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.DS);
            ImGui::NextColumn();

            ImGui::Text("ES (Extra Segment)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.ES);
            ImGui::NextColumn();

            ImGui::Text("SS (Stack Segment)");
            ImGui::NextColumn();
            ImGui::Text("%04X", regs.SS);
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Flags Register", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("FLAGS: %04X", regs.FLAGS);
            // ImGui::SameLine();
            ImGui::Separator();
            ImGui::Text("Binary: ");
            for (int i = 15; i >= 0; i--) {
                ImGui::SameLine();
                ImGui::Text("%d", (regs.FLAGS >> i) & 1);
                if (i % 4 == 0 && i > 0) {
                    ImGui::SameLine();
                    ImGui::Text(" ");
                }
            }

            ImGui::Separator();
            ImGui::Columns(3, "FlagsColumns");
            ImGui::Text("Flag");
            ImGui::NextColumn();
            ImGui::Text("Bit");
            ImGui::NextColumn();
            ImGui::Text("Status");
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::Text("CF (Carry)");
            ImGui::NextColumn();
            ImGui::Text("0");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.CF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("PF (Parity)");
            ImGui::NextColumn();
            ImGui::Text("2");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.PF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("AF (Auxiliary)");
            ImGui::NextColumn();
            ImGui::Text("4");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.AF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("ZF (Zero)");
            ImGui::NextColumn();
            ImGui::Text("6");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.ZF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("SF (Sign)");
            ImGui::NextColumn();
            ImGui::Text("7");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.SF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("TF (Trap)");
            ImGui::NextColumn();
            ImGui::Text("8");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.TF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("IF (Interrupt)");
            ImGui::NextColumn();
            ImGui::Text("9");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.IF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("DF (Direction)");
            ImGui::NextColumn();
            ImGui::Text("10");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.DF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Text("OF (Overflow)");
            ImGui::NextColumn();
            ImGui::Text("11");
            ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.OF) ? "SET" : "CLEAR");
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        ImGui::End();
    }
}

void GUIApplication::renderMemoryWindow() {
    if (!emulator)
        return;

    if (ImGui::Begin("Memory Viewer")) {
        const auto& memory = emulator->getMemory();

        ImGui::Text("Memory Size: %zu bytes", memory.size());

        ImGui::Text("Start Address:");
        ImGui::SameLine();
        int displayAddr = memoryViewStart;
        if (ImGui::InputInt(
                "##StartAddr", &displayAddr, 16, 256, ImGuiInputTextFlags_CharsHexadecimal)) {
            memoryViewStart =
                std::max(0, std::min(displayAddr, (int)memory.size() - memoryViewSize));
        }

        // ImGui::SameLine();
        ImGui::Separator();
        ImGui::Text("Size:");
        ImGui::SameLine();
        ImGui::SliderInt("##ViewSize", &memoryViewSize, 64, 1024);

        ImGui::Separator();

        if (ImGui::Button("Page Up")) {
            memoryViewStart = std::max(0, memoryViewStart - memoryViewSize);
        }
        ImGui::SameLine();
        if (ImGui::Button("Page Down")) {
            memoryViewStart =
                std::min((int)memory.size() - memoryViewSize, memoryViewStart + memoryViewSize);
        }
        ImGui::SameLine();
        if (ImGui::Button("Go to 0x0000")) {
            memoryViewStart = 0;
        }

        ImGui::BeginChild(
            "MemoryDisplay", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        ImGuiListClipper clipper;
        int rowsToShow = memoryViewSize / 16;
        clipper.Begin(rowsToShow);

        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int baseAddr = memoryViewStart + (row * 16);
                if (baseAddr >= (int)memory.size())
                    break;

                ImGui::Text("%04X:", baseAddr);
                ImGui::SameLine();

                for (int col = 0; col < 16; col++) {
                    int addr = baseAddr + col;
                    if (addr >= (int)memory.size()) {
                        ImGui::Text("   ");
                    } else {
                        uint8_t byte = emulator->readMemoryByte(addr);
                        ImGui::Text("%02X", byte);
                    }
                    if (col < 15) {
                        ImGui::SameLine();
                        if (col % 4 == 3) {
                            ImGui::Text(" ");
                            ImGui::SameLine();
                        }
                    }
                }

                ImGui::SameLine();
                // ImGui::Text(" |");
                ImGui::SameLine();

                for (int col = 0; col < 16; col++) {
                    int addr = baseAddr + col;
                    if (addr >= (int)memory.size()) {
                        ImGui::Text(" ");
                    } else {
                        uint8_t byte = emulator->readMemoryByte(addr);
                        char c = (byte >= 32 && byte < 127) ? (char)byte : '.';
                        ImGui::Text("%c", c);
                    }
                    if (col < 15)
                        ImGui::SameLine();
                }
                // ImGui::Text("|");
            }
        }

        ImGui::EndChild();

        ImGui::End();
    }
}

void GUIApplication::syncEditorBuffer() {
    assemblyEditorBuffer.clear();
    for (size_t i = 0; i < assemblyLines.size(); ++i) {
        assemblyEditorBuffer += assemblyLines[i];
        if (i < assemblyLines.size() - 1) {
            assemblyEditorBuffer += "\n";
        }
    }

    size_t copySize = std::min(assemblyEditorBuffer.size(), EDITOR_BUFFER_SIZE - 1);
    memcpy(assemblyEditorCharBuffer, assemblyEditorBuffer.c_str(), copySize);
    assemblyEditorCharBuffer[copySize] = '\0';
}

void GUIApplication::updateAssemblyLinesFromBuffer() {
    assemblyLines.clear();
    std::stringstream ss(assemblyEditorBuffer);
    std::string line;
    while (std::getline(ss, line)) {
        assemblyLines.push_back(line);
    }

    if (assemblyLines.empty()) {
        assemblyLines.push_back("");
    }
}

bool GUIApplication::saveAssemblyFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return false;
    }

    for (const auto& line : assemblyLines) {
        file << line << "\n";
    }

    file.close();
    std::cout << "Saved " << assemblyLines.size() << " lines to " << filePath << std::endl;
    return true;
}

void GUIApplication::assembleAndLoad() {
    if (!emulator) {
        std::cerr << "Emulator not initialized" << std::endl;
        return;
    }

    try {
        updateAssemblyLinesFromBuffer();
        emulator->reset();
        emulator->loadProgram(assemblyLines);
        std::cout << "Program assembled and loaded successfully (" << assemblyLines.size()
                  << " lines)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Assembly/load error: " << e.what() << std::endl;
    }
}

int GUIApplication::getCurrentLineNumber() {
    return currentLine + 1;
}

int GUIApplication::textEditCallback(ImGuiInputTextCallbackData* data) {
    GUIApplication* app = (GUIApplication*)data->UserData;

    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
        app->currentLine = 0;
        for (int i = 0; i < data->CursorPos; i++) {
            if (data->Buf[i] == '\n') {
                app->currentLine++;
            }
        }
    }

    return 0;
}

void GUIApplication::cleanupImGui() {
    if (imguiContext) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imguiContext);
        imguiContext = nullptr;
    }
}

void GUIApplication::cleanupOpenGL() {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }
}

void GUIApplication::cleanupSDL() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void GUIApplication::renderStackWindow() {
    if (!emulator) {
        return;
    }

    if (ImGui::Begin("Stack Viewer")) {
        const auto& registers = emulator->getRegisters();
        uint16_t sp = registers.SP;
        uint16_t ss = registers.SS;

        ImGui::Text("Stack Segment: 0x%04X", ss);
        ImGui::Text("Stack Pointer: 0x%04X", sp);
        ImGui::Text("Stack Top Address: 0x%05X", (ss << 4) + sp);

        ImGui::Separator();

        ImGui::SliderInt("View Size", &stackViewSize, 8, 64);

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text("Note: *SP (Stack Pointer)");
        ImGui::PopStyleColor();

        ImGui::Separator();

        if (ImGui::BeginTable("Stack", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Offset");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("ASCII");
            ImGui::TableHeadersRow();

            for (int i = -stackViewSize; i <= stackViewSize; i += 2) {
                uint16_t addr = static_cast<uint16_t>(static_cast<int>(sp) + i);
                uint32_t physAddr = (ss << 4) + addr;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (i == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::Text("0x%05X", physAddr);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Text("0x%05X", physAddr);
                }

                ImGui::TableNextColumn();
                if (i == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::Text("SP+%d", i);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Text("SP%+d", i);
                }

                ImGui::TableNextColumn();
                try {
                    uint16_t value = emulator->readMemoryWord(addr);
                    if (i == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                        ImGui::Text("0x%04X", value);
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::Text("0x%04X", value);
                    }

                    ImGui::TableNextColumn();

                    char ascii[3] = {0};
                    uint8_t low = value & 0xFF;
                    uint8_t high = (value >> 8) & 0xFF;
                    ascii[0] = (low >= 32 && low < 127) ? low : '.';
                    ascii[1] = (high >= 32 && high < 127) ? high : '.';
                    ImGui::Text("%s", ascii);
                } catch (...) {
                    ImGui::Text("????");
                    ImGui::TableNextColumn();
                    ImGui::Text("??");
                }
            }

            ImGui::EndTable();
        }

        // ImGui::Separator();
    }
    ImGui::End();
}

void GUIApplication::renderFileDialog() {
    if (!showFileDialog) {
        return;
    }

    std::string selectedFile = openFileDialog();
    if (!selectedFile.empty()) {
        if (loadAssemblyFile(selectedFile)) {
            syncEditorBuffer();
            assemblyEditorModified = false;
        }
        showFileDialog = false;
        return;
    }

    if (ImGui::Begin("Open Assembly File", &showFileDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char pathBuffer[512] = "./";
        static std::vector<std::string> files;
        static std::vector<std::string> directories;
        static bool needRefresh = true;

        if (needRefresh) {
            files.clear();
            directories.clear();

            const char* testFiles[] = {"samples/sample_01.txt",
                                       "samples/sample_02.txt",
                                       "samples/sample_03.txt",
                                       "samples/tui/sample_01.asm",
                                       "samples/tui/sample_02.asm",
                                       "samples/tui/test_ide.asm"};

            for (const char* file : testFiles) {
                std::ifstream test(file);
                if (test.good()) {
                    files.push_back(file);
                }
            }

            needRefresh = false;
        }

        ImGui::Text("Current directory: %s", pathBuffer);
        ImGui::Separator();

        ImGui::Text("Assembly Files:");
        for (size_t i = 0; i < files.size(); ++i) {
            if (ImGui::Selectable(files[i].c_str())) {
                if (loadAssemblyFile(files[i])) {
                    syncEditorBuffer();
                    assemblyEditorModified = false;
                }
                showFileDialog = false;
                break;
            }
        }

        ImGui::Separator();
        ImGui::Text("Enter file path manually:");
        static char manualPath[512] = "";
        if (ImGui::InputText("##manualpath",
                             manualPath,
                             sizeof(manualPath),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(manualPath) > 0) {
                if (loadAssemblyFile(manualPath)) {
                    syncEditorBuffer();
                    assemblyEditorModified = false;
                }
                showFileDialog = false;
            }
        }

        if (ImGui::Button("Cancel")) {
            showFileDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            needRefresh = true;
        }
        
        ImGui::End();
    }
}

std::string GUIApplication::openFileDialog(const std::string& title, const std::string& filters) {
#ifdef _WIN32

    OPENFILENAMEA ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Assembly Files\0*.asm;*.as;*.s\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
    return "";
#else

    std::string command = "zenity --file-selection --title=\"" + title + "\"";
    if (!filters.empty()) {
        command += " --file-filter='Assembly files (" + filters + ")|" + filters + "'";
        command += " --file-filter='All files|*'";
    }

    std::string result = executeCommand(command);
    if (!result.empty()) {
        return result;
    }

    command = "kdialog --getopenfilename . \"" + filters + "|Assembly files\"";
    result = executeCommand(command);
    return result;
#endif
}

bool GUIApplication::isFileDialogAvailable() {
#ifdef _WIN32

    return true;
#else
    return commandExists("which zenity") || commandExists("which kdialog");
#endif
}
