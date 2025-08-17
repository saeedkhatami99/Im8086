#include "gui/gui_application.h"
#include "emulator8086.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>  // for setenv

GUIApplication::GUIApplication() {
    // Constructor - minimal initialization
}

GUIApplication::~GUIApplication() {
    shutdown();
}

bool GUIApplication::initialize(int width, int height, const std::string& title) {
    if (initialized) {
        return true;
    }
    
    // Check if software rendering is already requested via environment
    const char* software_env = std::getenv("LIBGL_ALWAYS_SOFTWARE");
    bool force_software = (software_env && std::string(software_env) == "1");
    
    if (force_software) {
        std::cout << "Software rendering requested via LIBGL_ALWAYS_SOFTWARE" << std::endl;
    }
    
    try {
        if (!initSDL(width, height, title)) {
            throw std::runtime_error("Failed to initialize SDL2");
        }
        
        if (!initOpenGL()) {
            if (!force_software) {
                // If hardware OpenGL fails and we haven't tried software yet, try software rendering
                std::cout << "Hardware OpenGL failed, trying software rendering..." << std::endl;
                std::cout << "Note: Software rendering will be slower but should work on any system." << std::endl;
                
                // Clean up current attempt
                if (glContext) {
                    SDL_GL_DeleteContext(glContext);
                    glContext = nullptr;
                }
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                }
                SDL_Quit();
                
                // Set environment variable for software rendering
                setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
                
                // Try again with software rendering
                if (!initSDL(width, height, title)) {
                    throw std::runtime_error("Failed to initialize SDL2 with software rendering");
                }
                
                if (!initOpenGL()) {
                    throw std::runtime_error("Failed to initialize OpenGL even with software rendering");
                }
                
                std::cout << "Successfully initialized with software rendering" << std::endl;
            } else {
                throw std::runtime_error("Failed to initialize OpenGL with software rendering");
            }
        }
        
        if (!initImGui()) {
            throw std::runtime_error("Failed to initialize ImGui");
        }
        
        // Initialize emulator
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
    
    // Clear previous content
    assemblyLines.clear();
    
    // Read the file line by line
    std::string line;
    while (std::getline(file, line)) {
        assemblyLines.push_back(line);
    }
    
    file.close();
    
    if (assemblyLines.empty()) {
        std::cerr << "Warning: File is empty: " << filePath << std::endl;
        return false;
    }
    
    // Store the file path
    loadedFilePath = filePath;
    
    // Load the program into the emulator
    if (emulator) {
        try {
            emulator->reset();  // Reset emulator state
            emulator->loadProgram(assemblyLines);
            std::cout << "Successfully loaded " << assemblyLines.size() 
                      << " lines from " << filePath << std::endl;
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
    
    // Cleanup in reverse order of initialization
    emulator.reset();
    cleanupImGui();
    cleanupOpenGL();
    cleanupSDL();
    
    initialized = false;
    std::cout << "GUI Application shutdown complete\n";
}

bool GUIApplication::initSDL(int width, int height, const std::string& title) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL2 initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Try different OpenGL configurations in order of preference
    struct OpenGLConfig {
        int major, minor;
        int profile;
        int flags;
        const char* description;
    };
    
    OpenGLConfig configs[] = {
        // Try OpenGL 3.3 Core first (preferred)
        {3, 3, SDL_GL_CONTEXT_PROFILE_CORE, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, "OpenGL 3.3 Core"},
        // Fallback to OpenGL 3.0 Core
        {3, 0, SDL_GL_CONTEXT_PROFILE_CORE, 0, "OpenGL 3.0 Core"},
        // Fallback to OpenGL 2.1 Compatibility (for older drivers)
        {2, 1, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, 0, "OpenGL 2.1 Compatibility"},
        // Last resort: any OpenGL
        {0, 0, 0, 0, "OpenGL Legacy"}
    };
    
    window = nullptr;
    
    for (auto& config : configs) {
        std::cout << "Trying " << config.description << "..." << std::endl;
        
        // Set OpenGL attributes
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
        
        // Set buffer attributes
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        
        // Try to create window
        window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        );
        
        if (window) {
            std::cout << "Successfully created window with " << config.description << std::endl;
            break;
        } else {
            std::cout << "Failed to create window with " << config.description 
                      << ": " << SDL_GetError() << std::endl;
        }
    }
    
    if (!window) {
        std::cerr << "Failed to create SDL window with any OpenGL configuration" << std::endl;
        return false;
    }
    
    return true;
}

bool GUIApplication::initOpenGL() {
    // Try to create OpenGL context with fallback versions
    struct OpenGLContextConfig {
        int major, minor;
        const char* description;
    };
    
    OpenGLContextConfig contexts[] = {
        {3, 3, "OpenGL 3.3"},
        {3, 0, "OpenGL 3.0"},
        {2, 1, "OpenGL 2.1"},
        {0, 0, "OpenGL Legacy"}  // Let SDL choose
    };
    
    for (auto& config : contexts) {
        std::cout << "Trying to create " << config.description << " context..." << std::endl;
        
        // Set the context version if specified
        if (config.major > 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.minor);
        }
        
        // Create OpenGL context
        glContext = SDL_GL_CreateContext(window);
        if (glContext) {
            std::cout << "Successfully created " << config.description << " context" << std::endl;
            break;
        } else {
            std::cout << "Failed to create " << config.description << " context: " << SDL_GetError() << std::endl;
        }
    }
    
    if (!glContext) {
        std::cerr << "Failed to create any OpenGL context" << std::endl;
        
        // Try to get some debug information
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
    
    // Make context current
    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        std::cerr << "Failed to make OpenGL context current: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Enable vsync (adaptive if available, otherwise regular)
    if (SDL_GL_SetSwapInterval(-1) != 0) {
        // Adaptive vsync not supported, try regular vsync
        if (SDL_GL_SetSwapInterval(1) != 0) {
            std::cout << "Warning: VSync not supported" << std::endl;
        }
    }
    
    // Set viewport
    int displayW, displayH;
    SDL_GetWindowSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    
    // Print OpenGL info for debugging
    std::cout << "OpenGL context created successfully" << std::endl;
    
    return true;
}

bool GUIApplication::initImGui() {
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Note: Docking support requires ImGui docking branch or newer version
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup platform/renderer backends
    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext)) {
        std::cerr << "Failed to initialize ImGui SDL2 backend\n";
        return false;
    }
    
    // Detect OpenGL version and use appropriate GLSL version
    const char* glsl_version = nullptr;
    
    // Get OpenGL version from the current context
    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    
    std::cout << "Detected OpenGL " << major << "." << minor << std::endl;
    
    // Choose GLSL version based on OpenGL version
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
        // OpenGL 2.1 or older
        glsl_version = "#version 120";
        std::cout << "Using GLSL 120 (OpenGL 2.1)" << std::endl;
    }
    
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend with " << glsl_version << std::endl;
        
        // Try fallback to the oldest supported version
        if (glsl_version != "#version 120") {
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
        // Let ImGui handle the event first
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
    // Handle global key shortcuts
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
            // Toggle fullscreen (future implementation)
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
    }
}

void GUIApplication::handleWindowEvent(const SDL_Event& event) {
    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        int width = event.window.data1;
        int height = event.window.data2;
        glViewport(0, 0, width, height);
    }
}

void GUIApplication::update() {
    // Update emulator state if needed
    // This will be expanded in future weeks
}

void GUIApplication::render() {
    // Clear screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Render ImGui content
    renderImGui();
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Swap buffers
    SDL_GL_SwapWindow(window);
}

void GUIApplication::renderImGui() {
    renderMainMenuBar();
    renderDemoWindow();
    renderAssemblyEditor();
    renderEmulatorStatus();
    renderRegistersWindow();
    renderMemoryWindow();
}

void GUIApplication::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                // Future: New program
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // Future: Open assembly file
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // Future: Save assembly file
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
                    // Reload program if one was loaded
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
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Registers", "F2", &showRegistersWindow);
            ImGui::MenuItem("Memory", "F3", &showMemoryWindow);
            ImGui::MenuItem("Assembly", "F4", &showAssemblyEditor);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &showDemoWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // Future: About dialog
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void GUIApplication::renderDemoWindow() {
    // Show a simple demo window for Week 1
    if (!showDemoWindow) return;
    
    if (ImGui::Begin("8086 Emulator - Week 1 Demo", &showDemoWindow)) {
        
        ImGui::Text("SDL2 + ImGui Integration Success!");
        ImGui::Separator();
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
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
    if (!showAssemblyEditor) return;
    
    if (ImGui::Begin("Assembly Code", &showAssemblyEditor)) {
        
        if (!loadedFilePath.empty()) {
            ImGui::Text("File: %s", loadedFilePath.c_str());
            ImGui::Text("Lines: %zu", assemblyLines.size());
            ImGui::Separator();
            
            // Create a child window for the code with scrolling
            ImGui::BeginChild("AssemblyCode", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
            
            for (size_t i = 0; i < assemblyLines.size(); ++i) {
                ImGui::Text("%3zu: %s", i + 1, assemblyLines[i].c_str());
            }
            
            ImGui::EndChild();
            
            if (ImGui::Button("Reload File")) {
                if (!loadAssemblyFile(loadedFilePath)) {
                    std::cerr << "Failed to reload file: " << loadedFilePath << std::endl;
                }
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Reset Emulator")) {
                if (emulator) {
                    emulator->reset();
                    if (!assemblyLines.empty()) {
                        emulator->loadProgram(assemblyLines);
                    }
                    std::cout << "Emulator reset and program reloaded\n";
                }
            }
        } else {
            ImGui::Text("No assembly file loaded");
            ImGui::Text("Use: ./8086emu --gui <filename.asm>");
            ImGui::Separator();
            
            static char filename[256] = "";
            ImGui::InputText("File path", filename, sizeof(filename));
            ImGui::SameLine();
            if (ImGui::Button("Load File")) {
                if (strlen(filename) > 0) {
                    if (!loadAssemblyFile(filename)) {
                        std::cerr << "Failed to load file: " << filename << std::endl;
                    }
                }
            }
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
        
        // Show basic emulator info
        ImGui::Text("Memory Size: %zu bytes", emulator->getMemory().size());
        ImGui::Text("Current IP: %04X", emulator->getIP());
        
        // Program information
        const auto& program = emulator->getProgram();
        ImGui::Text("Program Lines: %zu", program.size());
        
        if (!program.empty() && emulator->getIP() < program.size()) {
            ImGui::Text("Current Instruction: %s", program[emulator->getIP()].c_str());
        }
        
        ImGui::Separator();
        
        // Execution controls
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
    if (!showRegistersWindow || !emulator) return;
    
    if (ImGui::Begin("Registers", &showRegistersWindow)) {
        const auto& regs = emulator->getRegisters();
        
        ImGui::Text("8086 CPU Registers");
        ImGui::Separator();
        
        // General Purpose Registers
        if (ImGui::CollapsingHeader("General Purpose Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(3, "RegColumns");
            ImGui::Text("Register"); ImGui::NextColumn();
            ImGui::Text("16-bit"); ImGui::NextColumn();
            ImGui::Text("8-bit (H/L)"); ImGui::NextColumn();
            ImGui::Separator();
            
            ImGui::Text("AX"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.AX.x); ImGui::NextColumn();
            ImGui::Text("AH=%02X  AL=%02X", regs.AX.bytes.h, regs.AX.bytes.l); ImGui::NextColumn();
            
            ImGui::Text("BX"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.BX.x); ImGui::NextColumn();
            ImGui::Text("BH=%02X  BL=%02X", regs.BX.bytes.h, regs.BX.bytes.l); ImGui::NextColumn();
            
            ImGui::Text("CX"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.CX.x); ImGui::NextColumn();
            ImGui::Text("CH=%02X  CL=%02X", regs.CX.bytes.h, regs.CX.bytes.l); ImGui::NextColumn();
            
            ImGui::Text("DX"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.DX.x); ImGui::NextColumn();
            ImGui::Text("DH=%02X  DL=%02X", regs.DX.bytes.h, regs.DX.bytes.l); ImGui::NextColumn();
            
            ImGui::Columns(1);
        }
        
        // Index and Pointer Registers
        if (ImGui::CollapsingHeader("Index & Pointer Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, "IndexColumns");
            ImGui::Text("Register"); ImGui::NextColumn();
            ImGui::Text("Value"); ImGui::NextColumn();
            ImGui::Separator();
            
            ImGui::Text("SI (Source Index)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.SI); ImGui::NextColumn();
            
            ImGui::Text("DI (Destination Index)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.DI); ImGui::NextColumn();
            
            ImGui::Text("BP (Base Pointer)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.BP); ImGui::NextColumn();
            
            ImGui::Text("SP (Stack Pointer)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.SP); ImGui::NextColumn();
            
            ImGui::Text("IP (Instruction Pointer)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.IP); ImGui::NextColumn();
            
            ImGui::Columns(1);
        }
        
        // Segment Registers
        if (ImGui::CollapsingHeader("Segment Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, "SegmentColumns");
            ImGui::Text("Register"); ImGui::NextColumn();
            ImGui::Text("Value"); ImGui::NextColumn();
            ImGui::Separator();
            
            ImGui::Text("CS (Code Segment)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.CS); ImGui::NextColumn();
            
            ImGui::Text("DS (Data Segment)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.DS); ImGui::NextColumn();
            
            ImGui::Text("ES (Extra Segment)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.ES); ImGui::NextColumn();
            
            ImGui::Text("SS (Stack Segment)"); ImGui::NextColumn();
            ImGui::Text("%04X", regs.SS); ImGui::NextColumn();
            
            ImGui::Columns(1);
        }
        
        // Flags Register
        if (ImGui::CollapsingHeader("Flags Register", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("FLAGS: %04X", regs.FLAGS);
            ImGui::SameLine();
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
            ImGui::Text("Flag"); ImGui::NextColumn();
            ImGui::Text("Bit"); ImGui::NextColumn();
            ImGui::Text("Status"); ImGui::NextColumn();
            ImGui::Separator();
            
            ImGui::Text("CF (Carry)"); ImGui::NextColumn();
            ImGui::Text("0"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.CF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("PF (Parity)"); ImGui::NextColumn();
            ImGui::Text("2"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.PF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("AF (Auxiliary)"); ImGui::NextColumn();
            ImGui::Text("4"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.AF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("ZF (Zero)"); ImGui::NextColumn();
            ImGui::Text("6"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.ZF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("SF (Sign)"); ImGui::NextColumn();
            ImGui::Text("7"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.SF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("TF (Trap)"); ImGui::NextColumn();
            ImGui::Text("8"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.TF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("IF (Interrupt)"); ImGui::NextColumn();
            ImGui::Text("9"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.IF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("DF (Direction)"); ImGui::NextColumn();
            ImGui::Text("10"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.DF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Text("OF (Overflow)"); ImGui::NextColumn();
            ImGui::Text("11"); ImGui::NextColumn();
            ImGui::Text("%s", (regs.FLAGS & regs.OF) ? "SET" : "CLEAR"); ImGui::NextColumn();
            
            ImGui::Columns(1);
        }
        
        ImGui::End();
    }
}

void GUIApplication::renderMemoryWindow() {
    if (!showMemoryWindow || !emulator) return;
    
    if (ImGui::Begin("Memory Viewer", &showMemoryWindow)) {
        const auto& memory = emulator->getMemory();
        
        ImGui::Text("Memory Size: %zu bytes", memory.size());
        
        // Navigation controls
        ImGui::Text("Start Address:");
        ImGui::SameLine();
        int displayAddr = memoryViewStart;
        if (ImGui::InputInt("##StartAddr", &displayAddr, 16, 256, ImGuiInputTextFlags_CharsHexadecimal)) {
            memoryViewStart = std::max(0, std::min(displayAddr, (int)memory.size() - memoryViewSize));
        }
        
        ImGui::SameLine();
        ImGui::Text("Size:");
        ImGui::SameLine();
        ImGui::SliderInt("##ViewSize", &memoryViewSize, 64, 1024);
        
        ImGui::Separator();
        
        // Memory display
        ImGui::BeginChild("MemoryDisplay", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        ImGuiListClipper clipper;
        int rowsToShow = memoryViewSize / 16;
        clipper.Begin(rowsToShow);
        
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                int baseAddr = memoryViewStart + (row * 16);
                if (baseAddr >= (int)memory.size()) break;
                
                // Address column
                ImGui::Text("%04X:", baseAddr);
                ImGui::SameLine();
                
                // Hex bytes
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
                ImGui::Text(" |");
                ImGui::SameLine();
                
                // ASCII representation
                for (int col = 0; col < 16; col++) {
                    int addr = baseAddr + col;
                    if (addr >= (int)memory.size()) {
                        ImGui::Text(" ");
                    } else {
                        uint8_t byte = emulator->readMemoryByte(addr);
                        char c = (byte >= 32 && byte < 127) ? (char)byte : '.';
                        ImGui::Text("%c", c);
                    }
                    if (col < 15) ImGui::SameLine();
                }
                ImGui::Text("|");
            }
        }
        
        ImGui::EndChild();
        
        // Navigation buttons
        if (ImGui::Button("Page Up")) {
            memoryViewStart = std::max(0, memoryViewStart - memoryViewSize);
        }
        ImGui::SameLine();
        if (ImGui::Button("Page Down")) {
            memoryViewStart = std::min((int)memory.size() - memoryViewSize, memoryViewStart + memoryViewSize);
        }
        ImGui::SameLine();
        if (ImGui::Button("Go to 0x0000")) {
            memoryViewStart = 0;
        }
        
        ImGui::End();
    }
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
