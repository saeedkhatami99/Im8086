cmake_minimum_required(VERSION 3.16)
project(Im8086 
    VERSION 1.2.0
    DESCRIPTION "Intel 8086 Microprocessor Emulator with GUI and TUI"
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_STATIC "Build static executable" OFF)
option(BUILD_PORTABLE "Build portable executable with bundled libraries" OFF)
set(TARGET_NAME "Im8086")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if(BUILD_STATIC AND NOT WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "Static build enabled - maximum portability")
endif()

if (MSVC)
  add_compile_options(/W4 /permissive-)
  add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX VC_EXTRALEAN NOGDI)
  if(BUILD_STATIC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)
find_package(PkgConfig)
find_package(SDL2 REQUIRED)

if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED CONFIG)
endif()

if(NOT SDL2_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(SDL2 REQUIRED sdl2)
endif()

if(PkgConfig_FOUND)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
endif()

if(NOT SDL2_IMAGE_FOUND)
    find_package(SDL2_image REQUIRED)
endif()

if(NOT SDL2_IMAGE_FOUND AND APPLE)
    message(STATUS "Debugging SDL2_image detection on macOS...")
    
    execute_process(COMMAND ls -la /opt/homebrew/include/SDL2/SDL_image.h
                   OUTPUT_VARIABLE HOMEBREW_ARM_HEADER
                   ERROR_QUIET)
    execute_process(COMMAND ls -la /usr/local/include/SDL2/SDL_image.h
                   OUTPUT_VARIABLE HOMEBREW_INTEL_HEADER
                   ERROR_QUIET)
    execute_process(COMMAND ls -la /opt/homebrew/include/SDL_image.h
                   OUTPUT_VARIABLE HOMEBREW_ARM_HEADER_DIRECT
                   ERROR_QUIET)
    execute_process(COMMAND ls -la /usr/local/include/SDL_image.h
                   OUTPUT_VARIABLE HOMEBREW_INTEL_HEADER_DIRECT
                   ERROR_QUIET)
    
    if(HOMEBREW_ARM_HEADER)
        message(STATUS "Found: /opt/homebrew/include/SDL2/SDL_image.h")
    endif()
    if(HOMEBREW_INTEL_HEADER)
        message(STATUS "Found: /usr/local/include/SDL2/SDL_image.h")
    endif()
    if(HOMEBREW_ARM_HEADER_DIRECT)
        message(STATUS "Found: /opt/homebrew/include/SDL_image.h")
    endif()
    if(HOMEBREW_INTEL_HEADER_DIRECT)
        message(STATUS "Found: /usr/local/include/SDL_image.h")
    endif()
    
    execute_process(COMMAND ls -la /opt/homebrew/lib/libSDL2_image*
                   OUTPUT_VARIABLE HOMEBREW_ARM_LIB
                   ERROR_QUIET)
    execute_process(COMMAND ls -la /usr/local/lib/libSDL2_image*
                   OUTPUT_VARIABLE HOMEBREW_INTEL_LIB
                   ERROR_QUIET)
    
    if(HOMEBREW_ARM_LIB)
        message(STATUS "Found ARM Homebrew SDL2_image libs: ${HOMEBREW_ARM_LIB}")
    endif()
    if(HOMEBREW_INTEL_LIB)
        message(STATUS "Found Intel Homebrew SDL2_image libs: ${HOMEBREW_INTEL_LIB}")
    endif()
    
    execute_process(
        COMMAND brew --prefix sdl2_image
        OUTPUT_VARIABLE SDL2_IMAGE_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    if(SDL2_IMAGE_PREFIX)
        message(STATUS "Homebrew SDL2_image prefix: ${SDL2_IMAGE_PREFIX}")
        execute_process(COMMAND ls -la ${SDL2_IMAGE_PREFIX}/include
                       OUTPUT_VARIABLE PREFIX_INCLUDE_CONTENTS
                       ERROR_QUIET)
        execute_process(COMMAND ls -la ${SDL2_IMAGE_PREFIX}/lib
                       OUTPUT_VARIABLE PREFIX_LIB_CONTENTS
                       ERROR_QUIET)
        if(PREFIX_INCLUDE_CONTENTS)
            message(STATUS "Contents of ${SDL2_IMAGE_PREFIX}/include: ${PREFIX_INCLUDE_CONTENTS}")
        endif()
        if(PREFIX_LIB_CONTENTS)
            message(STATUS "Contents of ${SDL2_IMAGE_PREFIX}/lib: ${PREFIX_LIB_CONTENTS}")
        endif()
        
        find_path(SDL2_IMAGE_INCLUDE_DIRS SDL_image.h
            PATHS 
                ${SDL2_IMAGE_PREFIX}/include
                ${SDL2_IMAGE_PREFIX}/include/SDL2
            NO_DEFAULT_PATH
        )
        
        if(NOT SDL2_IMAGE_INCLUDE_DIRS)
            find_path(SDL2_IMAGE_INCLUDE_DIRS SDL_image.h
                PATHS ${SDL2_IMAGE_PREFIX}/include/SDL2
                NO_DEFAULT_PATH
            )
        endif()
        
        find_library(SDL2_IMAGE_LIBRARIES
            NAMES SDL2_image libSDL2_image
            PATHS ${SDL2_IMAGE_PREFIX}/lib
            NO_DEFAULT_PATH
        )
        
        if(SDL2_IMAGE_INCLUDE_DIRS AND SDL2_IMAGE_LIBRARIES)
            set(SDL2_IMAGE_FOUND TRUE)
            message(STATUS "Found SDL2_image via Homebrew: ${SDL2_IMAGE_LIBRARIES}")
        endif()
    endif()
    
    if(NOT SDL2_IMAGE_FOUND)
        find_library(SDL2_IMAGE_LIBRARIES
            NAMES SDL2_image libSDL2_image
            PATHS 
                /usr/local/lib
                /opt/homebrew/lib
                /usr/lib
        )
        
        find_path(SDL2_IMAGE_INCLUDE_DIRS SDL_image.h
            PATHS 
                /usr/local/include
                /usr/local/include/SDL2
                /opt/homebrew/include
                /opt/homebrew/include/SDL2
                /usr/include
                /usr/include/SDL2
        )
        
        if(SDL2_IMAGE_INCLUDE_DIRS AND SDL2_IMAGE_LIBRARIES)
            set(SDL2_IMAGE_FOUND TRUE)
            message(STATUS "Found SDL2_image in system paths: ${SDL2_IMAGE_LIBRARIES}")
        endif()
    endif()
endif()

if(NOT SDL2_IMAGE_FOUND)
    find_path(SDL2_IMAGE_INCLUDE_DIRS SDL_image.h
        HINTS
        ENV SDL2_IMAGE_DIR
        PATH_SUFFIXES include SDL2 include/SDL2
        PATHS
        ${CMAKE_PREFIX_PATH}
        /usr/local
        /usr
        /opt/homebrew
        /usr/local/Cellar/sdl2_image
    )

    find_library(SDL2_IMAGE_LIBRARIES
        NAMES SDL2_image SDL2_image-2.0
        HINTS
        ENV SDL2_IMAGE_DIR
        PATH_SUFFIXES lib lib64 lib/x64 lib/x86
        PATHS
        ${CMAKE_PREFIX_PATH}
        /usr/local
        /usr
        /opt/homebrew/lib
        /usr/local/lib
    )
    
    if(SDL2_IMAGE_INCLUDE_DIRS AND SDL2_IMAGE_LIBRARIES)
        set(SDL2_IMAGE_FOUND TRUE)
        message(STATUS "Found SDL2_image manually: ${SDL2_IMAGE_LIBRARIES}")
        if(APPLE)
            message(STATUS "SDL2_image raw libraries: ${SDL2_IMAGE_LIBRARIES}")
            foreach(lib ${SDL2_IMAGE_LIBRARIES})
                if(lib MATCHES "SDL2_image")
                    set(SDL2_IMAGE_MAIN_LIB ${lib})
                    break()
                endif()
            endforeach()
            if(SDL2_IMAGE_MAIN_LIB)
                set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_MAIN_LIB})
                message(STATUS "Using SDL2_image library: ${SDL2_IMAGE_LIBRARIES}")
            endif()
        endif()
    endif()
endif()

if(NOT SDL2_FOUND AND WIN32)
    
    find_path(SDL2_INCLUDE_DIRS SDL.h
        HINTS
        ENV SDL2DIR
        PATH_SUFFIXES include SDL2 include/SDL2
        PATHS
        ${CMAKE_PREFIX_PATH}
        /usr/local
        /usr
    )

    find_library(SDL2_LIBRARIES
        NAMES SDL2 SDL2main
        HINTS
        ENV SDL2DIR
        PATH_SUFFIXES lib lib64 lib/x64 lib/x86
        PATHS
        ${CMAKE_PREFIX_PATH}
        /usr/local
        /usr
    )
    
    if(SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
        set(SDL2_FOUND TRUE)
        message(STATUS "Found SDL2 (Windows): ${SDL2_LIBRARIES}")
    endif()
endif()

if(BUILD_STATIC AND SDL2_FOUND)
    if(WIN32)
        
        find_library(SDL2_STATIC_LIB
            NAMES SDL2-static SDL2main
            HINTS ${SDL2_LIBRARY_DIRS}
            PATH_SUFFIXES lib lib64
        )
        if(SDL2_STATIC_LIB)
            set(SDL2_LIBRARIES ${SDL2_STATIC_LIB})
            message(STATUS "Using static SDL2: ${SDL2_STATIC_LIB}")
        endif()
    elseif(UNIX)
        
        set(SDL2_STATIC_DEPS pthread dl m rt)
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            list(APPEND SDL2_STATIC_DEPS asound pulse X11 Xext Xcursor Xinerama Xi Xrandr Xss Xxf86vm)
        endif()
    endif()
endif()

find_package(OpenGL QUIET)
set(BUILD_GUI OFF)

if(SDL2_FOUND AND OPENGL_FOUND)
    set(BUILD_GUI ON)
    message(STATUS "GUI support enabled")
else()
    message(STATUS "GUI support disabled (SDL2 or OpenGL not found)")
endif()

set(BUILD_TUI OFF)

if(UNIX)
    if(PkgConfig_FOUND)
        pkg_check_modules(NCURSES QUIET ncurses)
    endif()
    
    
    if(NOT NCURSES_FOUND)
        find_package(Curses QUIET)
        if(CURSES_FOUND)
            set(NCURSES_FOUND TRUE)
            set(NCURSES_LIBRARIES ${CURSES_LIBRARIES})
            set(NCURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIRS})
        endif()
    endif()
    
    if(NCURSES_FOUND)
        set(BUILD_TUI ON)
        message(STATUS "TUI support enabled")
    else()
        message(STATUS "TUI support disabled (ncurses not found)")
    endif()
else()
    find_library(PDCURSES_LIB pdcurses)
    if(PDCURSES_LIB)
        set(BUILD_TUI ON)
        set(NCURSES_LIBRARIES ${PDCURSES_LIB})
        message(STATUS "TUI support enabled (PDCurses)")
    else()
        message(STATUS "TUI support disabled (PDCurses not found)")
    endif()
endif()

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
        )
        
        if(TARGET SDL2::SDL2)
            target_link_libraries(imgui PUBLIC SDL2::SDL2)
            target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
        elseif(SDL2_INCLUDE_DIRS)
            target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
        endif()
        
        if(SDL2_CFLAGS_OTHER)
            target_compile_definitions(imgui PUBLIC ${SDL2_CFLAGS_OTHER})
        endif()
    else()
        message(FATAL_ERROR "ImGui not found in ${IMGUI_DIR}. Run ./setup_gui.sh first.")
    endif()
endif()

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

set(MAIN_SOURCES
    src/main.cpp
)
set(TUI_SOURCES)

if(BUILD_TUI)
    list(APPEND TUI_SOURCES
        src/tui.cpp
        src/ide_tui.cpp
    )
endif()

set(GUI_SOURCES)

if(BUILD_GUI)
    list(APPEND GUI_SOURCES
        src/gui/gui_application.cpp
        src/gui/main_gui.cpp
        src/gui/splash_window.cpp
        src/image_loader.cpp
    )
endif()

add_executable(${TARGET_NAME} 
    ${MAIN_SOURCES} 
    ${CORE_SOURCES}
    ${TUI_SOURCES}
    ${GUI_SOURCES}
)

if(BUILD_TUI)
    target_compile_definitions(${TARGET_NAME} PRIVATE WITH_TUI)
endif()

if(BUILD_GUI)
    target_compile_definitions(${TARGET_NAME} PRIVATE WITH_GUI)
endif()

set(LINK_LIBRARIES)

if(BUILD_TUI)
    list(APPEND LINK_LIBRARIES ${NCURSES_LIBRARIES})
    if(NCURSES_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${NCURSES_INCLUDE_DIRS})
    endif()
endif()

if(BUILD_GUI)
    list(APPEND LINK_LIBRARIES 
        imgui
        ${OPENGL_LIBRARIES}
    )
    
    if(SDL2_LIBRARY_DIRS)
        target_link_directories(${TARGET_NAME} PRIVATE ${SDL2_LIBRARY_DIRS})
        message(STATUS "Added SDL2 library directories: ${SDL2_LIBRARY_DIRS}")
    endif()
    
    if(TARGET SDL2::SDL2)
        list(APPEND LINK_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    elseif(SDL2_LIBRARIES)
        list(APPEND LINK_LIBRARIES ${SDL2_LIBRARIES})
    endif()
    
    if(SDL2_IMAGE_FOUND)
        message(STATUS "SDL2_image linking debug:")
        if(TARGET SDL2_image::SDL2_image)
            message(STATUS "  TARGET SDL2_image::SDL2_image exists: YES")
        else()
            message(STATUS "  TARGET SDL2_image::SDL2_image exists: NO")
        endif()
        message(STATUS "  SDL2_IMAGE_LIBRARIES: ${SDL2_IMAGE_LIBRARIES}")
        message(STATUS "  SDL2_IMAGE_LIBRARY_DIRS: ${SDL2_IMAGE_LIBRARY_DIRS}")
        message(STATUS "  SDL2_IMAGE_LDFLAGS: ${SDL2_IMAGE_LDFLAGS}")
        
        if(SDL2_IMAGE_LIBRARY_DIRS)
            target_link_directories(${TARGET_NAME} PRIVATE ${SDL2_IMAGE_LIBRARY_DIRS})
            message(STATUS "  Added library directories: ${SDL2_IMAGE_LIBRARY_DIRS}")
        endif()
        
        if(TARGET SDL2_image::SDL2_image)
            list(APPEND LINK_LIBRARIES SDL2_image::SDL2_image)
            message(STATUS "  Using target: SDL2_image::SDL2_image")
        elseif(SDL2_IMAGE_LIBRARIES)
            if(APPLE AND SDL2_IMAGE_LIBRARIES MATCHES ";")
                message(STATUS "  Detected multiple libraries on macOS: ${SDL2_IMAGE_LIBRARIES}")
                foreach(lib ${SDL2_IMAGE_LIBRARIES})
                    message(STATUS "  Checking library: ${lib}")
                    if(lib MATCHES "SDL2_image" AND NOT lib STREQUAL "SDL2")
                        list(APPEND LINK_LIBRARIES ${lib})
                        message(STATUS "  Adding SDL2_image library: ${lib}")
                        break()
                    endif()
                endforeach()
            else()
                list(APPEND LINK_LIBRARIES ${SDL2_IMAGE_LIBRARIES})
                message(STATUS "  Adding all SDL2_image libraries: ${SDL2_IMAGE_LIBRARIES}")
            endif()
        endif()
        add_compile_definitions(SDL2_IMAGE_FOUND)
    endif()
    
    target_include_directories(${TARGET_NAME} PRIVATE 
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )

    if(SDL2_CFLAGS_OTHER)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
    endif()
    
    if(WIN32)
        list(APPEND LINK_LIBRARIES opengl32)
    elseif(APPLE)
        list(APPEND LINK_LIBRARIES "-framework OpenGL")
        target_compile_definitions(${TARGET_NAME} PRIVATE GL_SILENCE_DEPRECATION)
    else()
        list(APPEND LINK_LIBRARIES GL)
    endif()
endif()

if(LINK_LIBRARIES)
    target_link_libraries(${TARGET_NAME} ${LINK_LIBRARIES})
    if(BUILD_STATIC AND BUILD_GUI AND SDL2_FOUND)
        if(WIN32)
            
            target_link_libraries(${TARGET_NAME} winmm setupapi version imm32)
        elseif(UNIX AND SDL2_STATIC_DEPS)
            
            target_link_libraries(${TARGET_NAME} ${SDL2_STATIC_DEPS})
        endif()
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${TARGET_NAME} PRIVATE 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
    target_link_options(${TARGET_NAME} PRIVATE 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
endif()

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    ${CMAKE_BINARY_DIR}/resources
    COMMENT "Copying resources folder to build directory"
)

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    DESTINATION .
    COMPONENT Runtime
)

add_custom_target(run
    COMMAND ${TARGET_NAME}
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${TARGET_NAME} in REPL mode"
)

option(BUILD_TESTS "Build test suite" ON)

if(BUILD_TESTS)
    add_executable(test_emulator
        tests/test_emulator.cpp
        ${CORE_SOURCES}
    )
    target_include_directories(test_emulator PRIVATE include tests)
    
    if(BUILD_GUI)
        add_executable(test_gui
            tests/test_gui.cpp
            src/gui/gui_application.cpp
            src/gui/splash_window.cpp
            src/image_loader.cpp
            ${CORE_SOURCES}
        )
        target_include_directories(test_gui PRIVATE 
            include 
            tests
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
        )
        target_link_libraries(test_gui 
            imgui
            ${OPENGL_LIBRARIES}
        )
        
        
        if(TARGET SDL2::SDL2)
            target_link_libraries(test_gui SDL2::SDL2 SDL2::SDL2main)
        elseif(SDL2_LIBRARIES)
            target_link_libraries(test_gui ${SDL2_LIBRARIES})
        endif()
        
        if(SDL2_IMAGE_FOUND)
            if(SDL2_IMAGE_LIBRARY_DIRS)
                target_link_directories(test_gui PRIVATE ${SDL2_IMAGE_LIBRARY_DIRS})
            endif()
            
            if(TARGET SDL2_image::SDL2_image)
                target_link_libraries(test_gui SDL2_image::SDL2_image)
            elseif(SDL2_IMAGE_LIBRARIES)
                target_link_libraries(test_gui ${SDL2_IMAGE_LIBRARIES})
            endif()
            target_compile_definitions(test_gui PRIVATE SDL2_IMAGE_FOUND)
        endif()
        
        if(SDL2_CFLAGS_OTHER)
            target_compile_definitions(test_gui PRIVATE ${SDL2_CFLAGS_OTHER})
        endif()
        if(WIN32)
            target_link_libraries(test_gui opengl32)
        elseif(APPLE)
            target_link_libraries(test_gui "-framework OpenGL")
            target_compile_definitions(test_gui PRIVATE GL_SILENCE_DEPRECATION)
        else()
            target_link_libraries(test_gui GL)
        endif()
        
        add_custom_command(TARGET test_gui POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/resources
            COMMENT "Copying resources folder for test_gui"
        )
    endif()
    
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

add_custom_target(run-tui
    COMMAND echo "Usage: make run-tui FILE=samples/sample_01.txt"
    COMMENT "Run TUI mode with a file (specify FILE=<path>)"
)

add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E tar czf ${TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}.tar.gz $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating distribution package"
)

message(STATUS "===== Build Configuration =====")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Target name: ${TARGET_NAME}")
message(STATUS "TUI support: ${BUILD_TUI}")
message(STATUS "GUI support: ${BUILD_GUI}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

if(BUILD_GUI)
    message(STATUS "SDL2 found: ${SDL2_FOUND}")
    message(STATUS "SDL2_image found: ${SDL2_IMAGE_FOUND}")
    if(SDL2_IMAGE_FOUND)
        if(TARGET SDL2_image::SDL2_image)
            message(STATUS "SDL2_image target: SDL2_image::SDL2_image")
        elseif(SDL2_IMAGE_LIBRARIES)
            message(STATUS "SDL2_image libraries: ${SDL2_IMAGE_LIBRARIES}")
        endif()
    endif()
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
