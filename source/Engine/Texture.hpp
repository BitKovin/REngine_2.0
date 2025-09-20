#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Renderer/RHI/RenderInterface.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstring>

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
    Texture(const unsigned char* data, int width, int height, uint32_t format = RenderInterface::RGBA, bool generateMipmaps = true) {
        loadFromRawData(data, width, height, format, generateMipmaps);
    }

    ~Texture() {
        if (textureID != RenderInterface::INVALID_HANDLE)
            RenderInterface::DestroyTexture(textureID);
    }

    void bind() const {
        RenderInterface::BindTexture(RenderInterface::TEXTURE_2D, textureID);
    }

    bool valid = false;

    uint32_t getID() const {
        return textureID;
    }

private:
    uint32_t textureID = RenderInterface::INVALID_HANDLE;

    static inline bool isPowerOfTwo(int v) { return v > 0 && ((v & (v - 1)) == 0); }

    void setupTexture(int width, int height, uint32_t format, const void* pixels, bool generateMipmaps) {
        if (width <= 0 || height <= 0) return;
        if (!pixels) return;

        // Convert format to bgfx format
        uint32_t bgfxFormat = ConvertToBgfxFormat(format);
        
        // Create bgfx memory from pixel data
        void* mem = RenderInterface::Alloc(width * height * GetBytesPerPixel(format));
        memcpy(mem, pixels, width * height * GetBytesPerPixel(format));
        
        // Create texture with bgfx
        textureID = RenderInterface::CreateTexture2D(width, height, generateMipmaps, 1, bgfxFormat, 0, mem);
        
        if (textureID == RenderInterface::INVALID_HANDLE) {
            std::cerr << "Failed to create texture with bgfx" << std::endl;
            RenderInterface::Free(mem);
            return;
        }
        
        RenderInterface::Free(mem);
        valid = true;
    }
    
    // Convert OpenGL format to bgfx format
    uint32_t ConvertToBgfxFormat(uint32_t glFormat) {
        switch (glFormat) {
            case RenderInterface::RGBA:
                return 0; // BGFX_TEXTURE_FORMAT_RGBA8
            case RenderInterface::RGB:
                return 1; // BGFX_TEXTURE_FORMAT_RGB8
            default:
                return 0; // Default to RGBA8
        }
    }
    
    // Get bytes per pixel for format
    int GetBytesPerPixel(uint32_t format) {
        switch (format) {
            case RenderInterface::RGBA:
                return 4;
            case RenderInterface::RGB:
                return 3;
            default:
                return 4;
        }
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

        setupTexture(converted_surface->w, converted_surface->h, RenderInterface::RGBA, converted_surface->pixels, generateMipmaps);
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

        setupTexture(converted_surface->w, converted_surface->h, RenderInterface::RGBA, converted_surface->pixels, generateMipmaps);
        SDL_FreeSurface(converted_surface);
    }

    void loadFromRawData(const unsigned char* data, int width, int height, uint32_t format, bool generateMipmaps) {
        // quick sanity check for commonly expected RGB buffer size
        if (!data) {
            std::cerr << "loadFromRawData: null data pointer\n";
            return;
        }

        setupTexture(width, height, format, data, generateMipmaps);
    }
};
