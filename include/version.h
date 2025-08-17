#ifndef VERSION_H
#define VERSION_H

#define PROJECT_NAME "Im8086"
#define PROJECT_DESCRIPTION "Intel 8086 Microprocessor Emulator"
#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 0
#define PROJECT_VERSION_PATCH 0
#define PROJECT_VERSION_STRING "1.0.0"

#ifdef _WIN32
#define BUILD_PLATFORM "Windows"
#elif defined(__APPLE__)
#define BUILD_PLATFORM "macOS"
#elif defined(__linux__)
#define BUILD_PLATFORM "Linux"
#else
#define BUILD_PLATFORM "Unknown"
#endif

#ifdef _DEBUG
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__ " " __TIME__
#endif

#endif
