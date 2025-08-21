#include "image_loader.h"

#ifdef _WIN32
#include <io.h>
#include <commdlg.h>
#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif
#ifdef SDL2_IMAGE_FOUND
#ifdef __APPLE__
    #include <SDL2/SDL_image.h>
#else
    #include <SDL2/SDL_image.h>
#endif
#endif
#include <iostream>
#include <fstream>

GLuint ImageLoader::createTextureFromRGBA(unsigned char* data, int width, int height) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    return textureID;
}

SDL_Surface* ImageLoader::loadImageSurface(const std::string& filename) {
    std::vector<std::string> paths = {
        "./resources/" + filename
    };
    
    for (const auto& path : paths) {
        SDL_Surface* surface = nullptr;
        
#ifdef SDL2_IMAGE_FOUND
        surface = IMG_Load(path.c_str());
        if (surface) {
            std::cout << "Successfully loaded image with SDL_image from: " << path << std::endl;
            return surface;
        }
#endif
        
        surface = SDL_LoadBMP(path.c_str());
        if (surface) {
            std::cout << "Successfully loaded BMP image from: " << path << std::endl;
            return surface;
        }
    }
    
    std::cerr << "Failed to load image: " << filename << " - tried multiple paths and formats" << std::endl;
    return nullptr;
}

Image ImageLoader::loadImage(const std::string& filepath) {
    Image image;
    SDL_Surface* surface = loadImageSurface(filepath);
    if (!surface) {
        std::string pngPath = filepath;
        size_t lastDot = pngPath.find_last_of('.');
        if (lastDot != std::string::npos) {
            pngPath = pngPath.substr(0, lastDot) + ".png";
        } else {
            pngPath += ".png";
        }
        surface = loadImageSurface(pngPath);
    }
    
    if (!surface) {
        std::string bmpPath = filepath;
        size_t lastDot = bmpPath.find_last_of('.');
        if (lastDot != std::string::npos) {
            bmpPath = bmpPath.substr(0, lastDot) + ".bmp";
        } else {
            bmpPath += ".bmp";
        }
        surface = loadImageSurface(bmpPath);
    }
    
    if (!surface) {
        return image;
    }
    
    SDL_Surface* rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    
    if (!rgbaSurface) {
        std::cerr << "Failed to convert surface format: " << SDL_GetError() << std::endl;
        return image;
    }
    
    image.width = rgbaSurface->w;
    image.height = rgbaSurface->h;
    image.textureID = createTextureFromRGBA((unsigned char*)rgbaSurface->pixels, image.width, image.height);
    image.loaded = true;
    
    SDL_FreeSurface(rgbaSurface);
    
    return image;
}

void ImageLoader::unloadImage(Image& image) {
    if (image.loaded && image.textureID != 0) {
        glDeleteTextures(1, &image.textureID);
        image.textureID = 0;
        image.loaded = false;
        image.width = 0;
        image.height = 0;
    }
}

SDL_Surface* ImageLoader::loadImageForIcon(const std::string& filename) {
    SDL_Surface* surface = loadImageSurface(filename);
    if (!surface) {
        std::string pngPath = filename;
        size_t lastDot = pngPath.find_last_of('.');
        if (lastDot != std::string::npos) {
            pngPath = pngPath.substr(0, lastDot) + ".png";
        } else {
            pngPath += ".png";
        }
        surface = loadImageSurface(pngPath);
    }
    
    if (!surface) {
        std::string bmpPath = filename;
        size_t lastDot = bmpPath.find_last_of('.');
        if (lastDot != std::string::npos) {
            bmpPath = bmpPath.substr(0, lastDot) + ".bmp";
        } else {
            bmpPath += ".bmp";
        }
        surface = loadImageSurface(bmpPath);
    }
    
    if (!surface) {
        std::cerr << "Failed to load icon image: " << filename << std::endl;
        return nullptr;
    }

    SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    
    if (!convertedSurface) {
        std::cerr << "Failed to convert icon surface format: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    return convertedSurface;
}
