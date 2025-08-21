#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include "platform_opengl.h"

#include <SDL2/SDL.h>
#include <vector>
#include <string>

struct Image {
    GLuint textureID = 0;
    int width = 0;
    int height = 0;
    bool loaded = false;
};

class ImageLoader {
public:
    static Image loadImage(const std::string& filepath);
    static void unloadImage(Image& image);
    static SDL_Surface* loadImageForIcon(const std::string& filename);
private:
    static GLuint createTextureFromRGBA(unsigned char* data, int width, int height);
    static SDL_Surface* loadImageSurface(const std::string& filename);
};

#endif
