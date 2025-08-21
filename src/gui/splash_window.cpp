#ifdef _WIN32
#include "opengl_modern.h"
#else
#include "platform_opengl.h"
#endif

#include "gui/splash_window.h"
#include "image_loader.h"
#include "theme_detector.h"

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
    
    std::string themeLogo;
    if (logoPath.empty()) {
        themeLogo = ThemeDetector::getThemeAwareLogo("./resources");
    } else {
        if (logoPath.find('/') == std::string::npos && logoPath.find('\\') == std::string::npos) {
            if (logoPath == "Logo.png") {
                themeLogo = ThemeDetector::getThemeAwareLogo("./resources");
            } else {
                themeLogo = "./resources/" + logoPath;
            }
        } else {
            themeLogo = logoPath;
        }
    }
    
    logoSurface = IMG_Load(themeLogo.c_str());
    if (!logoSurface) {
        std::cerr << "Failed to load splash logo: " << themeLogo << " - " << IMG_GetError() << std::endl;
        
        if (themeLogo != "./resources/Logo.png") {
            std::cerr << "Falling back to default logo..." << std::endl;
            logoSurface = IMG_Load("./resources/Logo.png");
        }
        
        if (!logoSurface) {
            std::cerr << "Failed to load fallback logo as well!" << std::endl;
            return false;
        }
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
    
    ThemeDetector::Theme currentTheme = ThemeDetector::detectSystemTheme();
    
    Uint8 alphaValue = (Uint8)(alpha * 255);
    Uint8 bgAlpha = alphaValue;
    
    Uint8 bgR, bgG, bgB;
    if (currentTheme == ThemeDetector::Theme::LIGHT) {
        bgR = (Uint8)(255 * alpha);
        bgG = (Uint8)(255 * alpha);
        bgB = (Uint8)(255 * alpha);
    } else {
        bgR = (Uint8)(0 * alpha);
        bgG = (Uint8)(0 * alpha);
        bgB = (Uint8)(0 * alpha);
    }

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
    
    applyRoundedCorners(15);
    
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

void SplashWindow::applyRoundedCorners(int cornerRadius) {
    if (!windowSurface) return;
    
    ThemeDetector::Theme currentTheme = ThemeDetector::detectSystemTheme();
    
    Uint32 maskColor = SDL_MapRGBA(windowSurface->format, 0, 0, 0, 0);
    
    int w = windowSurface->w;
    int h = windowSurface->h;
    
    for (int y = 0; y < cornerRadius; y++) {
        for (int x = 0; x < cornerRadius; x++) {
            int dx = cornerRadius - x;
            int dy = cornerRadius - y;
            if (dx * dx + dy * dy > cornerRadius * cornerRadius) {
                if (y < h && x < w) {
                    Uint32* pixel = (Uint32*)((Uint8*)windowSurface->pixels + y * windowSurface->pitch + x * windowSurface->format->BytesPerPixel);
                    *pixel = maskColor;
                }
            }
        }
    }
    
    for (int y = 0; y < cornerRadius; y++) {
        for (int x = w - cornerRadius; x < w; x++) {
            int dx = x - (w - cornerRadius);
            int dy = cornerRadius - y;
            if (dx * dx + dy * dy > cornerRadius * cornerRadius) {
                if (y < h && x >= 0) {
                    Uint32* pixel = (Uint32*)((Uint8*)windowSurface->pixels + y * windowSurface->pitch + x * windowSurface->format->BytesPerPixel);
                    *pixel = maskColor;
                }
            }
        }
    }
    
    for (int y = h - cornerRadius; y < h; y++) {
        for (int x = 0; x < cornerRadius; x++) {
            int dx = cornerRadius - x;
            int dy = y - (h - cornerRadius);
            if (dx * dx + dy * dy > cornerRadius * cornerRadius) {
                if (y >= 0 && x < w) {
                    Uint32* pixel = (Uint32*)((Uint8*)windowSurface->pixels + y * windowSurface->pitch + x * windowSurface->format->BytesPerPixel);
                    *pixel = maskColor;
                }
            }
        }
    }
    
    for (int y = h - cornerRadius; y < h; y++) {
        for (int x = w - cornerRadius; x < w; x++) {
            int dx = x - (w - cornerRadius);
            int dy = y - (h - cornerRadius);
            if (dx * dx + dy * dy > cornerRadius * cornerRadius) {
                if (y >= 0 && x >= 0) {
                    Uint32* pixel = (Uint32*)((Uint8*)windowSurface->pixels + y * windowSurface->pitch + x * windowSurface->format->BytesPerPixel);
                    *pixel = maskColor;
                }
            }
        }
    }
}

void SplashWindow::drawRoundedRect(SDL_Surface* surface, int x, int y, int w, int h, int radius, Uint32 color) {
    if (!surface) return;
    
    SDL_Rect topRect = {x + radius, y, w - 2 * radius, radius};
    SDL_Rect middleRect = {x, y + radius, w, h - 2 * radius};
    SDL_Rect bottomRect = {x + radius, y + h - radius, w - 2 * radius, radius};
    
    SDL_FillRect(surface, &topRect, color);
    SDL_FillRect(surface, &middleRect, color);
    SDL_FillRect(surface, &bottomRect, color);
    
    for (int cy = 0; cy < radius; cy++) {
        for (int cx = 0; cx < radius; cx++) {
            int dx = radius - cx;
            int dy = radius - cy;
            if (dx * dx + dy * dy <= radius * radius) {
                if (x + cx >= 0 && y + cy >= 0 && x + cx < surface->w && y + cy < surface->h) {
                    Uint32* pixel = (Uint32*)((Uint8*)surface->pixels + (y + cy) * surface->pitch + (x + cx) * surface->format->BytesPerPixel);
                    *pixel = color;
                }
                if (x + w - radius + dx - 1 >= 0 && y + cy >= 0 && x + w - radius + dx - 1 < surface->w && y + cy < surface->h) {
                    Uint32* pixel = (Uint32*)((Uint8*)surface->pixels + (y + cy) * surface->pitch + (x + w - radius + dx - 1) * surface->format->BytesPerPixel);
                    *pixel = color;
                }
                if (x + cx >= 0 && y + h - radius + dy - 1 >= 0 && x + cx < surface->w && y + h - radius + dy - 1 < surface->h) {
                    Uint32* pixel = (Uint32*)((Uint8*)surface->pixels + (y + h - radius + dy - 1) * surface->pitch + (x + cx) * surface->format->BytesPerPixel);
                    *pixel = color;
                }
                if (x + w - radius + dx - 1 >= 0 && y + h - radius + dy - 1 >= 0 && x + w - radius + dx - 1 < surface->w && y + h - radius + dy - 1 < surface->h) {
                    Uint32* pixel = (Uint32*)((Uint8*)surface->pixels + (y + h - radius + dy - 1) * surface->pitch + (x + w - radius + dx - 1) * surface->format->BytesPerPixel);
                    *pixel = color;
                }
            }
        }
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
