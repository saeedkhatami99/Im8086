#ifndef WINDOWS_PLATFORM_H
#define WINDOWS_PLATFORM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
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

#define APIENTRY __stdcall
#define WINGDIAPI __declspec(dllimport)
#define CALLBACK __stdcall

#include <commdlg.h>

#include <GL/gl.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

#endif
