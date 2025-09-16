#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <iostream>
#include <vector>

#include "malloc_override.h"

class Texture {
public:
    Texture(const std::string& filename, bool generateMipmaps = true) {
        loadFromFile(filename, generateMipmaps);
    }

    Texture() 
    {
        
    }

    // Load from compressed data (PNG/JPEG in memory)
    Texture(const unsigned char* data, size_t size, bool generateMipmaps = true) {
        loadFromMemoryCompressed(data, size, generateMipmaps);
    }

    // Load from raw pixel data (RGBA or BGRA32)
    Texture(const unsigned char* data, int width, int height, GLenum format = GL_RGBA, bool generateMipmaps = true) {
        loadFromRawData(data, width, height, format, generateMipmaps);
    }

    ~Texture() {
        if (textureID != 0)
            glDeleteTextures(1, &textureID);
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    bool valid = false;

    GLuint getID() const {
        return textureID;
    }

private:
    GLuint textureID = 0;

    void setupTexture(GLint width, GLint height, GLenum format, const void* pixels, bool generateMipmaps) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Anisotropic filtering (if supported)
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        if (maxAniso > 0.0f) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        valid = true;
    }

    void loadFromFile(const std::string& filename, bool generateMipmaps) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << filename << "   Error loading image: " << IMG_GetError() << std::endl;
            return;
        }

        SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        if (!converted_surface) {
            std::cerr << filename << "  Error converting surface: " << SDL_GetError() << std::endl;
            return;
        }

        setupTexture(converted_surface->w, converted_surface->h, GL_RGBA, converted_surface->pixels, generateMipmaps);
        SDL_FreeSurface(converted_surface);
    }

    void loadFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps) {
        SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
        if (!rw) {
            std::cerr << "Error creating RWops: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_Surface* surface = IMG_Load_RW(rw, 1); // 1 = free RWops automatically
        if (!surface) {
            std::cerr << "Error loading image from memory: " << IMG_GetError() << std::endl;
            return;
        }

        SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        if (!converted_surface) {
            std::cerr << "Error converting surface from memory: " << SDL_GetError() << std::endl;
            return;
        }

        setupTexture(converted_surface->w, converted_surface->h, GL_RGBA, converted_surface->pixels, generateMipmaps);
        SDL_FreeSurface(converted_surface);
    }

    void loadFromRawData(const unsigned char* data, int width, int height, GLenum format, bool generateMipmaps) {
        setupTexture(width, height, format, data, generateMipmaps);
    }
};
