#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <iostream>

#include "malloc_override.h"

class Texture {
public:
    Texture(const std::string& filename, bool generateMipmaps = true) {
        loadFromFile(filename, generateMipmaps);
    }

    ~Texture() {
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

    void loadFromFile(const std::string& filename, bool generateMipmaps) 
    {

        

        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) 
        {
            std::cerr << filename << "   Error loading image: " << IMG_GetError() << std::endl;
            return;
        }

        SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        if (!converted_surface) {
            std::cerr << filename  << "  Error converting surface: " << SDL_GetError() << std::endl;
            return;
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted_surface->w, converted_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted_surface->pixels);



        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Anisotropic filtering (if supported)
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        if (maxAniso > 0.0f) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

#if __EMSCRIPTEN__

        //generateMipmaps = false;

#endif 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        SDL_FreeSurface(converted_surface);

        valid = true;

    }
};
