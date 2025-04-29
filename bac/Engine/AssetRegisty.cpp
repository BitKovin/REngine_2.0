#include "AssetRegisty.h"

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
