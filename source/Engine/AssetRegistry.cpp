#include "AssetRegistry.h"
#include "FileSystem/FileSystem.h"

std::unordered_map<std::string, Shader*> AssetRegistry::shaderCache;
std::unordered_map<std::string, Texture*> AssetRegistry::textureCache;
std::unordered_map<std::string, CubemapTexture*> AssetRegistry::textureCubeCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelAnimationCache;
std::unordered_map<std::string, TTF_Font*> AssetRegistry::fontCache;

void AssetRegistry::ClearMemory()
{

	for (auto texCube : textureCubeCache)
	{
		if (texCube.second == nullptr) continue;
		delete(texCube.second);
	}

	textureCubeCache.clear();

	for (auto tex : textureCache)
	{
		if (tex.second == nullptr) continue;
		delete(tex.second);
	}
	textureCache.clear();

	for (auto model : skinnedModelCache)
	{
		if (model.second == nullptr) continue;

		model.second->clear();
		delete(model.second);
	}
	skinnedModelCache.clear();

	for (auto model : skinnedModelAnimationCache)
	{
		if (model.second == nullptr) continue;

		model.second->clear();
		delete(model.second);
	}
	skinnedModelAnimationCache.clear();


}

void AssetRegistry::RegisterTexture(Texture* texture, string path)
{

	textureCache[path] = texture;

}

std::string AssetRegistry::ReadFileToString(string filename)
{
	return FileSystemEngine::ReadFile(filename);
}

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

roj::SkinnedModel* AssetRegistry::GetSkinnedAnimationFromFile(const string& path)
{
	auto it = skinnedModelAnimationCache.find(path);
	if (it != skinnedModelAnimationCache.end())
	{
		return it->second;
	}

	roj::ModelLoader<roj::SkinnedMesh> modelLoader;

	modelLoader.SkipVisual = true;

	modelLoader.load(path);

	Logger::Log(modelLoader.getInfoLog());

	skinnedModelAnimationCache[path] = new roj::SkinnedModel(modelLoader.get());


	return skinnedModelAnimationCache[path];
}
