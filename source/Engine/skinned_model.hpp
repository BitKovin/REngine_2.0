#ifndef SKINNED_MODEL_HPP
#define SKINNED_MODEL_HPP
#include "model.hpp"
#include <unordered_map>

#include <vector>

#include "Texture.hpp"

#include "BoundingSphere.hpp"
#include "BoundingBox.hpp"

#include "utility/hashed_string.hpp"

#define MAX_BONE_INFLUENCE 4

#define MAX_SKINNED_BONES 128

namespace roj
{
	
	struct SkinnedMesh
	{

		VertexBuffer* vertices;
		IndexBuffer* indices;
		VertexArrayObject* VAO;

		std::vector<MeshTexture> textures;

		std::vector<VertexData> vertexLocations;
		std::vector<uint32_t> vertexIndices;

		string materialName;
		string name;

		Texture* cachedBaseColor = nullptr;
		Texture* cachedEmissiveColor = nullptr;

		void DestroyBuffers()
		{
			if (vertices != nullptr)
			{
				delete(vertices);
			}

			vertices = nullptr;

			if (indices != nullptr)
			{
				delete(indices);
			}

			indices = nullptr;

			if (VAO != nullptr)
			{
				delete(VAO);
			}

			VAO = nullptr;
		}

	};



	struct BoneNode
	{
		hashed_string name;
		glm::mat4 transform;
		std::vector<BoneNode> children;

		uint16_t id; //for lookup optimization

	};

	struct FrameBoneTransform {
		std::vector<float> positionTimestamps = {};
		std::vector<float> rotationTimestamps = {};
		std::vector<float> scaleTimestamps = {};

		std::vector<glm::vec3> positions = {};
		std::vector<glm::quat> rotations = {};
		std::vector<glm::vec3> scales = {};
	};

	struct Animation {
		float duration = 0.0f;
		float ticksPerSec = 1.0f;
		float frameTime = 0.0;
		BoneNode rootBone;
		std::unordered_map<hashed_string, FrameBoneTransform> animationFrames = {};
	};


	struct BoneInfo
	{
		int id;
		glm::mat4 offset;
	};
	struct SkinnedModel
	{
		int boneCount{ 0 };
		aiCamera* sceneCamera;
		glm::mat4 globalInversed;
		std::vector<SkinnedMesh> meshes;
		std::unordered_map<hashed_string, BoneInfo> boneInfoMap;
		std::unordered_map<hashed_string, Animation> animations;

		BoneNode defaultRoot;
		std::unordered_map<hashed_string, BoneNode> boneNodesMap;
		std::unordered_map<hashed_string, hashed_string> parentMap;

		BoundingSphere boundingSphere;
		BoundingBox boundingBox;

		std::vector<SkinnedMesh>::iterator begin();
		std::vector<SkinnedMesh>::iterator end();
		void clear();
	};


}
#endif //-SKINNED_MODEL_HPP

