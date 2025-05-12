#include "AssetRegistry.h"

std::unordered_map<std::string, Shader*> AssetRegistry::shaderCache;
std::unordered_map<std::string, Texture*> AssetRegistry::textureCache;
std::unordered_map<std::string, CubemapTexture*> AssetRegistry::textureCubeCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelCache;
std::unordered_map<std::string, TTF_Font*> AssetRegistry::fontCache;

roj::SkinnedModel* AssetRegistry::GetSkinnedModelFromFile(const string& path)
{

    auto it = skinnedModelCache.find(path);
    if (it != skinnedModelCache.end())
    {
        return it->second;
    }

    roj::ModelLoader<roj::SkinnedMesh> modelLoader;

    modelLoader.load(path);

    Logger::Log(modelLoader.getInfoLog());

    skinnedModelCache[path] = new roj::SkinnedModel(modelLoader.get());


    return skinnedModelCache[path];
}
