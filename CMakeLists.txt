cmake_minimum_required(VERSION 3.16)
project(Im8086 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set the target name (was 8086emu in Makefile)
set(TARGET_NAME "8086emu")

# Compiler flags - matching original Makefile
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Warning flags - matching original Makefile
add_compile_options(-Wall -Wextra -Wpedantic)

# Include directories
include_directories(include)

# Find packages
find_package(PkgConfig REQUIRED)

# SDL2 for GUI
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    # Fallback for systems without SDL2 cmake support
    pkg_check_modules(SDL2 QUIET sdl2)
endif()

# OpenGL for GUI
find_package(OpenGL QUIET)

# Check if we should build GUI (requires SDL2 and OpenGL)
set(BUILD_GUI OFF)
if(SDL2_FOUND AND OPENGL_FOUND)
    set(BUILD_GUI ON)
    message(STATUS "GUI support enabled")
else()
    message(STATUS "GUI support disabled (SDL2 or OpenGL not found)")
endif()

# ncurses for TUI (Unix-like systems only)
set(BUILD_TUI OFF)
if(UNIX)
    pkg_check_modules(NCURSES QUIET ncurses)
    if(NCURSES_FOUND)
        set(BUILD_TUI ON)
        message(STATUS "TUI support enabled")
    else()
        message(STATUS "TUI support disabled (ncurses not found)")
    endif()
else()
    # Windows support would use PDCurses
    find_library(PDCURSES_LIB pdcurses)
    if(PDCURSES_LIB)
        set(BUILD_TUI ON)
        set(NCURSES_LIBRARIES ${PDCURSES_LIB})
        message(STATUS "TUI support enabled (PDCurses)")
    else()
        message(STATUS "TUI support disabled (PDCurses not found)")
    endif()
endif()

# ImGui sources (only if GUI is enabled)
if(BUILD_GUI)
    set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")
    if(EXISTS "${IMGUI_DIR}")
        set(IMGUI_SOURCES
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )
        
        add_library(imgui STATIC ${IMGUI_SOURCES})
        target_include_directories(imgui PUBLIC 
            ${IMGUI_DIR} 
            ${IMGUI_DIR}/backends
            ${SDL2_INCLUDE_DIRS}
        )
        if(SDL2_CFLAGS_OTHER)
            target_compile_definitions(imgui PUBLIC ${SDL2_CFLAGS_OTHER})
        endif()
    else()
        message(FATAL_ERROR "ImGui not found in ${IMGUI_DIR}. Run ./setup_gui.sh first.")
    endif()
endif()

# Core emulator sources (from original Makefile)
set(CORE_SOURCES
    src/emulator8086.cpp
    src/memory_components.cpp
    src/instructions/arithmetic.cpp
    src/instructions/bit_manipulation.cpp
    src/instructions/data_transfer.cpp
    src/instructions/logical.cpp
    src/instructions/processor_control.cpp
    src/instructions/program_transfer.cpp
    src/instructions/string.cpp
)

# Main executable source
set(MAIN_SOURCES
    src/main.cpp
)

# TUI sources (conditional)
set(TUI_SOURCES)
if(BUILD_TUI)
    list(APPEND TUI_SOURCES
        src/tui.cpp
        src/ide_tui.cpp
    )
endif()

# GUI sources (conditional)
set(GUI_SOURCES)
if(BUILD_GUI)
    list(APPEND GUI_SOURCES
        src/gui/gui_application.cpp
        src/gui/main_gui.cpp
    )
endif()

# Create main executable
add_executable(${TARGET_NAME} 
    ${MAIN_SOURCES} 
    ${CORE_SOURCES}
    ${TUI_SOURCES}
    ${GUI_SOURCES}
)

# Set compile definitions based on enabled features
if(BUILD_TUI)
    target_compile_definitions(${TARGET_NAME} PRIVATE WITH_TUI)
endif()

if(BUILD_GUI)
    target_compile_definitions(${TARGET_NAME} PRIVATE WITH_GUI)
endif()

# Link libraries based on enabled features
set(LINK_LIBRARIES)

# TUI libraries
if(BUILD_TUI)
    list(APPEND LINK_LIBRARIES ${NCURSES_LIBRARIES})
    if(NCURSES_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${NCURSES_INCLUDE_DIRS})
    endif()
endif()

# GUI libraries
if(BUILD_GUI)
    list(APPEND LINK_LIBRARIES 
        imgui
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
    )
    
    # Include directories for GUI
    target_include_directories(${TARGET_NAME} PRIVATE 
        ${SDL2_INCLUDE_DIRS}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    
    # Compile definitions
    if(SDL2_CFLAGS_OTHER)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
    endif()
    
    # Platform-specific OpenGL linking
    if(WIN32)
        list(APPEND LINK_LIBRARIES opengl32)
    elseif(APPLE)
        list(APPEND LINK_LIBRARIES "-framework OpenGL")
    else()
        list(APPEND LINK_LIBRARIES GL)
    endif()
endif()

# Apply all link libraries
if(LINK_LIBRARIES)
    target_link_libraries(${TARGET_NAME} ${LINK_LIBRARIES})
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${TARGET_NAME} PRIVATE 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
    target_link_options(${TARGET_NAME} PRIVATE 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
endif()

# Installation
install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
)

# Custom targets to match original Makefile functionality
add_custom_target(run
    COMMAND ${TARGET_NAME}
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${TARGET_NAME} in REPL mode"
)

# Test targets
option(BUILD_TESTS "Build test suite" ON)

if(BUILD_TESTS)
    # Test for core emulator
    add_executable(test_emulator
        tests/test_emulator.cpp
        ${CORE_SOURCES}
    )
    target_include_directories(test_emulator PRIVATE include tests)
    
    # Test for GUI (only if GUI is enabled)
    if(BUILD_GUI)
        add_executable(test_gui
            tests/test_gui.cpp
            src/gui/gui_application.cpp
            ${CORE_SOURCES}
        )
        target_include_directories(test_gui PRIVATE 
            include 
            tests
            ${SDL2_INCLUDE_DIRS}
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
        )
        target_link_libraries(test_gui 
            imgui
            ${SDL2_LIBRARIES}
            ${OPENGL_LIBRARIES}
        )
        if(SDL2_CFLAGS_OTHER)
            target_compile_definitions(test_gui PRIVATE ${SDL2_CFLAGS_OTHER})
        endif()
        if(WIN32)
            target_link_libraries(test_gui opengl32)
        elseif(APPLE)
            target_link_libraries(test_gui "-framework OpenGL")
        else()
            target_link_libraries(test_gui GL)
        endif()
    endif()
    
    # Test runner target
    add_custom_target(test
        COMMAND test_emulator
        DEPENDS test_emulator
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running emulator tests"
    )
    
    if(BUILD_GUI)
        add_custom_target(test-gui
            COMMAND test_gui
            DEPENDS test_gui
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running GUI tests"
        )
    endif()
    
    add_custom_target(test-all
        DEPENDS test_emulator $<$<BOOL:${BUILD_GUI}>:test_gui>
        COMMENT "Running all tests"
    )
endif()

add_custom_target(run-ide
    COMMAND ${TARGET_NAME} --ide
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${TARGET_NAME} in IDE mode"
)

if(BUILD_GUI)
    add_custom_target(run-gui
        COMMAND ${TARGET_NAME} --gui
        DEPENDS ${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running ${TARGET_NAME} in GUI mode"
    )
endif()

# TUI run target with file parameter
add_custom_target(run-tui
    COMMAND echo "Usage: make run-tui FILE=samples/sample_01.txt"
    COMMENT "Run TUI mode with a file (specify FILE=<path>)"
)

# Distribution target
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E tar czf ${TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}.tar.gz $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating distribution package"
)

# Print build configuration summary
message(STATUS "===== Build Configuration =====")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Target name: ${TARGET_NAME}")
message(STATUS "TUI support: ${BUILD_TUI}")
message(STATUS "GUI support: ${BUILD_GUI}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

if(BUILD_GUI)
    message(STATUS "SDL2 found: ${SDL2_FOUND}")
    message(STATUS "OpenGL found: ${OPENGL_FOUND}")
endif()

if(BUILD_TUI)
    if(UNIX)
        message(STATUS "ncurses found: ${NCURSES_FOUND}")
    else()
        message(STATUS "PDCurses library: ${PDCURSES_LIB}")
    endif()
endif()

message(STATUS "===============================")

# Help target
add_custom_target(show-help
    COMMAND ${CMAKE_COMMAND} -E echo "Available targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  all \\(default\\)   - Build executable"
    COMMAND ${CMAKE_COMMAND} -E echo "  run             - Run REPL mode"
    COMMAND ${CMAKE_COMMAND} -E echo "  run-ide         - Run IDE mode"
    COMMAND ${CMAKE_COMMAND} -E echo "  run-gui         - Run GUI mode \\(if enabled\\)"
    COMMAND ${CMAKE_COMMAND} -E echo "  run-tui         - Instructions for TUI mode"
    COMMAND ${CMAKE_COMMAND} -E echo "  test            - Run emulator tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-gui        - Run GUI tests \\(if enabled\\)"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-all        - Run all tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  dist            - Create distribution package"
    COMMAND ${CMAKE_COMMAND} -E echo "  install         - Install to system"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean           - Clean build files"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Build variables:"
    COMMAND ${CMAKE_COMMAND} -E echo "  CMAKE_BUILD_TYPE=Debug|Release"
    COMMAND ${CMAKE_COMMAND} -E echo "  BUILD_TESTS=ON|OFF"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Examples:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake -DCMAKE_BUILD_TYPE=Debug .."
    COMMAND ${CMAKE_COMMAND} -E echo "  make test"
    COMMAND ${CMAKE_COMMAND} -E echo "  make run-ide"
    COMMAND ${CMAKE_COMMAND} -E echo "  make dist"
    COMMENT "Showing help information"
)
