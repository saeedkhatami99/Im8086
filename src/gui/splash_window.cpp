#include "gui/splash_window.h"
#include "image_loader.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#define NOGDI
#endif

#ifdef __APPLE__
    #include <SDL2/SDL_image.h>
#else
    #include <SDL2/SDL_image.h>
#endif
#include <iostream>

SplashWindow::SplashWindow() : window(nullptr), windowSurface(nullptr), logoSurface(nullptr), visible(false),
                              fadeState(COMPLETE), animationStartTime(0), 
                              fadeInDuration(0.5f), holdDuration(1.5f), fadeOutDuration(0.5f) {
}

SplashWindow::~SplashWindow() {
    destroy();
}

bool SplashWindow::create(const std::string& logoPath, int height) {
    
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL for splash: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    
    std::string fullPath = "./resources/" + logoPath;
    logoSurface = IMG_Load(fullPath.c_str());
    if (!logoSurface) {
        std::cerr << "Failed to load splash logo: " << fullPath << " - " << IMG_GetError() << std::endl;
        return false;
    }
    
    float aspectRatio = (float)logoSurface->w / (float)logoSurface->h;
    int width = (int)(height * aspectRatio);
    
    int padding = 20;
    int windowWidth = width + (padding * 2);
    int windowHeight = height + (padding * 2);
    
    window = SDL_CreateWindow(
        "Im8086 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
    );
    
    if (!window) {
        std::cerr << "Failed to create splash window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        std::cerr << "Failed to get window surface: " << SDL_GetError() << std::endl;
        return false;
    }
    
    visible = true;
    render();
    
    return true;
}

void SplashWindow::render(float alpha) {
    if (!window || !windowSurface || !logoSurface) return;
    
    Uint8 alphaValue = (Uint8)(alpha * 255);
    Uint8 bgAlpha = alphaValue;
    Uint8 bgR = (Uint8)(45 * alpha);
    Uint8 bgG = (Uint8)(45 * alpha);
    Uint8 bgB = (Uint8)(48 * alpha);

    SDL_FillRect(windowSurface, nullptr, SDL_MapRGB(windowSurface->format, bgR, bgG, bgB));

    int padding = 20;
    float aspectRatio = (float)logoSurface->w / (float)logoSurface->h;
    int logoHeight = windowSurface->h - (padding * 2);
    int logoWidth = (int)(logoHeight * aspectRatio);

    if (logoWidth > windowSurface->w - (padding * 2)) {
        logoWidth = windowSurface->w - (padding * 2);
        logoHeight = (int)(logoWidth / aspectRatio);
    }
    
    int x = (windowSurface->w - logoWidth) / 2;
    int y = (windowSurface->h - logoHeight) / 2;
    
    SDL_Rect dstRect = {x, y, logoWidth, logoHeight};
    SDL_Surface* scaledLogo = SDL_CreateRGBSurface(0, logoWidth, logoHeight, 32, 
                                                   0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    
    if (scaledLogo) {
        SDL_BlitScaled(logoSurface, nullptr, scaledLogo, nullptr);
        
        SDL_LockSurface(scaledLogo);
        Uint32* pixels = (Uint32*)scaledLogo->pixels;
        int pixelCount = scaledLogo->w * scaledLogo->h;
        
        for (int i = 0; i < pixelCount; i++) {
            Uint32 pixel = pixels[i];
            Uint8 r = (pixel & 0x00FF0000) >> 16;
            Uint8 g = (pixel & 0x0000FF00) >> 8;
            Uint8 b = (pixel & 0x000000FF);
            Uint8 a = (pixel & 0xFF000000) >> 24;
            
            r = (Uint8)(r * alpha);
            g = (Uint8)(g * alpha);
            b = (Uint8)(b * alpha);
            
            pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        SDL_UnlockSurface(scaledLogo);
        
        SDL_BlitSurface(scaledLogo, nullptr, windowSurface, &dstRect);
        SDL_FreeSurface(scaledLogo);
    }
    
    SDL_UpdateWindowSurface(window);
}

void SplashWindow::show() {
    if (window) {
        SDL_ShowWindow(window);
        visible = true;
        render(1.0f);
    }
}

void SplashWindow::showWithFade(float fadeInTime, float holdTime, float fadeOutTime) {
    if (!window) return;
    
    fadeInDuration = fadeInTime;
    holdDuration = holdTime;
    fadeOutDuration = fadeOutTime;
    
    fadeState = FADE_IN;
    animationStartTime = SDL_GetTicks();
    
    SDL_ShowWindow(window);
    visible = true;
    
    SDL_Event event;
    while (fadeState != COMPLETE && visible) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                fadeState = COMPLETE;
                break;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                fadeState = COMPLETE;
                break;
            }
        }
        
        if (!updateFade()) {
            break;
        }
        
        float alpha = getCurrentAlpha();
        
        static int debugCounter = 0;
        if (debugCounter % 30 == 0) {
            const char* stateNames[] = {"FADE_IN", "HOLD", "FADE_OUT", "COMPLETE"};
            std::cout << "Fade state: " << stateNames[fadeState] << ", Alpha: " << alpha << std::endl;
        }
        debugCounter++;
        
        render(alpha);
        
        SDL_Delay(16);
    }
}

bool SplashWindow::updateFade() {
    Uint32 currentTime = SDL_GetTicks();
    float elapsed = (currentTime - animationStartTime) / 1000.0f;
    
    switch (fadeState) {
        case FADE_IN:
            if (elapsed >= fadeInDuration) {
                fadeState = HOLD;
                animationStartTime = currentTime;
            }
            break;
            
        case HOLD:
            if (elapsed >= holdDuration) {
                fadeState = FADE_OUT;
                animationStartTime = currentTime;
            }
            break;
            
        case FADE_OUT:
            if (elapsed >= fadeOutDuration) {
                fadeState = COMPLETE;
                return false;
            }
            break;
            
        case COMPLETE:
            return false;
    }
    
    return true;
}

float SplashWindow::getCurrentAlpha() {
    Uint32 currentTime = SDL_GetTicks();
    float elapsed = (currentTime - animationStartTime) / 1000.0f;
    
    switch (fadeState) {
        case FADE_IN:
            return elapsed / fadeInDuration;
            
        case HOLD:
            return 1.0f;
            
        case FADE_OUT:
            return 1.0f - (elapsed / fadeOutDuration);
            
        case COMPLETE:
            return 0.0f;
    }
    
    return 1.0f;
}

void SplashWindow::hide() {
    if (window) {
        SDL_HideWindow(window);
        visible = false;
    }
}

void SplashWindow::destroy() {
    if (logoSurface) {
        SDL_FreeSurface(logoSurface);
        logoSurface = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
        windowSurface = nullptr;
    }
    
    visible = false;
}
