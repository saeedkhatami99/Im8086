#pragma once

#ifdef __APPLE__
    #include <SDL2/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif
#include <string>

class SplashWindow {
public:
    SplashWindow();
    ~SplashWindow();
    
    bool create(const std::string& logoPath, int height = 300);
    void show();
    void showWithFade(float fadeInTime = 0.5f, float holdTime = 1.5f, float fadeOutTime = 0.5f);
    void hide();
    void destroy();
    
    bool isVisible() const { return visible; }
    
private:
    enum FadeState {
        FADE_IN,
        HOLD,
        FADE_OUT,
        COMPLETE
    };
    
    SDL_Window* window;
    SDL_Surface* windowSurface;
    SDL_Surface* logoSurface;
    bool visible;
    
    FadeState fadeState;
    Uint32 animationStartTime;
    float fadeInDuration;
    float holdDuration;
    float fadeOutDuration;
    
    void render(float alpha = 1.0f);
    void centerWindow();
    bool updateFade();
    float getCurrentAlpha();
    void applyRoundedCorners(int cornerRadius = 15);
    void drawRoundedRect(SDL_Surface* surface, int x, int y, int w, int h, int radius, Uint32 color);
};
