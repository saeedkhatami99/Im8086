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
    #ifndef NOSERVICE
    #define NOSERVICE
    #endif
    #ifndef NOMCX
    #define NOMCX
    #endif
    #ifndef NOIME
    #define NOIME
    #endif

    #include <windows.h>
    
    #include <commdlg.h>
    
    #ifdef APIENTRY
    #undef APIENTRY
    #endif
    #ifdef WINGDIAPI
    #undef WINGDIAPI
    #endif
    #ifdef CALLBACK
    #undef CALLBACK
    #endif
    #ifdef GLAPIENTRY
    #undef GLAPIENTRY
    #endif

    #define APIENTRY __stdcall
    #define WINGDIAPI __declspec(dllimport)
    #define CALLBACK __stdcall
    #define GLAPIENTRY APIENTRY

    #include <GL/gl.h>
    
    #ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
    #endif
    
    #define popen _popen
    #define pclose _pclose

#else
    #include "platform_opengl.h"
#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

inline bool initializeModernOpenGL() {
    #ifdef _WIN32
        #ifdef GLAD_GL_VERSION_3_3
            return gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
        #else
            return true;
        #endif
    #else
        return true;
    #endif
}

#endif
