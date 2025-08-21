#ifndef PLATFORM_OPENGL_H
#define PLATFORM_OPENGL_H

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
    #ifdef WINAPI
    #undef WINAPI
    #endif

    #define APIENTRY __stdcall
    #define WINGDIAPI __declspec(dllimport)
    #define CALLBACK __stdcall
    #define GLAPIENTRY APIENTRY

    #include <GL/gl.h>
    
    #define popen _popen
    #define pclose _pclose

#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

#endif
