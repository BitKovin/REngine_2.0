#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "malloc_override.h"

#include "Shader.hpp"
#include "skinned_model.hpp"
#include "model.hpp"
#include "Texture.hpp"
#include "TextureCube.hpp"
#include "Logger.hpp"

class AssetRegistry
{

private:
    static std::unordered_map<std::string, Shader*> shaderCache;
    static std::unordered_map<std::string, Texture*> textureCache;
    static std::unordered_map<std::string, CubemapTexture*> textureCubeCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelAnimationCache;

    static std::unordered_map<std::string, TTF_Font*> fontCache;

public:
	
    static void ClearMemory();

    static Shader* GetShaderByName(const std::string& name, ShaderType shaderType)
    {

        string fileEnding;

        switch (shaderType)
        {

        case(ShaderType::PixelShader):
                fileEnding = ".frag";
                break;
        case(ShaderType::VertexShader):
            fileEnding = ".vert";
            break;

        default:
            break;
        }


        string fileName = name + fileEnding;


        std::string key = fileName; // Unique key for each shader type

        // Check if the shader is already cached
        auto it = shaderCache.find(key);
        if (it != shaderCache.end())
        {
            return it->second; // Return cached shader
        }

        std::string filePath = "GameData/shaders/" + fileName;
        std::string shaderCode = ReadFileToString(filePath);

        // Cache the newly loaded shader
        shaderCache[key] = Shader::FromCode(shaderCode.c_str(), shaderType);

        return shaderCache[key];
    }

    static Texture* GetTextureFromFile(string filename);

    static CubemapTexture* GetTextureCubeFromFile(string filename);

    static void RegisterTexture(Texture* texture, string path);


    static TTF_Font* GetFontFromFile(const char* filename, int fontSize) {
        std::string key = std::string(filename) + "_" + std::to_string(fontSize);
        auto it = fontCache.find(key);
        if (it != fontCache.end()) {
            return it->second; // Return cached font.
        }

        TTF_Font* font = TTF_OpenFont(filename, fontSize);
        if (!font) {
            std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
            return nullptr;
        }

        

        fontCache[key] = font;
        return font;
    }

    static std::string ReadFileToString(string filename);

    static roj::SkinnedModel* GetSkinnedModelFromFile(const string& path);
    static roj::SkinnedModel* GetSkinnedAnimationFromFile(const string& path);

private:

};
