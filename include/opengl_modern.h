#ifndef OPENGL_MODERN_H
#define OPENGL_MODERN_H

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN
    #endif
    #ifndef NOGDI
    #define NOGDI
    #endif
    
    #include <windows.h>
    
    #ifdef APIENTRY
    #undef APIENTRY
    #endif
    #ifdef WINGDIAPI
    #undef WINGDIAPI
    #endif
#endif

#include <gl.h>

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

inline bool initializeModernOpenGL() {
    #ifdef GLAD_GL_VERSION_3_3
        return gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    #else
        return true;
    #endif
}

#endif
