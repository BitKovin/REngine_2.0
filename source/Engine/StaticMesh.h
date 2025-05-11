#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

#include "BSP/Quake3Bsp.h"

using namespace std;

class Entity;

class StaticMesh : public IDrawMesh
{
private:

	mat4 finalizedWorld;

	vec3 finalizedPosition = vec3(0);
	vec3 finalizedRotation = vec3(0);
	vec3 finalizedScale = vec3(1);



protected:

	virtual void ApplyAdditionalShaderParams(ShaderProgram* shader_program)
	{

	}

	string PixelShader = "default_pixel";

	ShaderProgram* forward_shader_program = nullptr;

	int numInstances = -1;

	void OnDispose()
	{

	}

	vec3 positionOffset = vec3();
	vec3 rotationOffset = vec3();

public:

	roj::SkinnedModel* model = nullptr;

	Entity* OwnerEntity = nullptr;

	Texture* ColorTexture = nullptr;

	vec3 Position = vec3(0);
	vec3 Rotation = vec3(0);

	vec3 Scale = vec3(1);

	string TexturesLocation = "GameData/textures/";

	bool DepthWrite = true;

	bool CastShadows = true;
	bool Visible = true;
	bool CastHiddenShadows = false;
	bool CastDetailShadows = false;


	StaticMesh(Entity* owner)
	{
		OwnerEntity = owner;
	}

	void SetPixelShader(string name)
	{
		PixelShader = name;

		forward_shader_program = nullptr;

	}

	mat4 GetWorldMatrix()
	{

		mat4 posOffset = translate(positionOffset);
		mat4 rotOffset = MathHelper::GetRotationMatrix(rotationOffset);

		return translate(Position) * rotOffset * MathHelper::GetRotationMatrix(Rotation) * scale(Scale) * posOffset;
	}

	LightVolPointData GetLightVolData();

	vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes()
	{
		vector<MeshUtils::PositionVerticesIndices> result;

		if (model == nullptr)
			return result;

		mat3 world = GetWorldMatrix();

		for (auto mesh : model->meshes)
		{

			MeshUtils::PositionVerticesIndices meshData;

			meshData.indices = mesh.vertexIndices;

			for (auto& vertex : mesh.vertexLocations)
			{
				meshData.vertices.push_back(world * vertex.Position);
			}

			result.push_back(meshData);

		}

		return result;
	}

	float GetDistanceToCamera()
	{
		return distance(Camera::position, Position) * (IsViewmodel ? 0.1 : 1);
	}

	void FinalizeFrameData()
	{
		finalizedWorld = GetWorldMatrix();
		finalizedPosition = Position;
		finalizedRotation = Rotation;
		finalizedScale = Scale;
	}



	//obj or gml files are strongly recommended
	virtual void LoadFromFile(const string& path)
	{

		model = AssetRegistry::GetSkinnedModelFromFile(path);

	}

	bool IsInFrustrum(Frustum frustrum)
	{

		if (model == nullptr) return false;

		auto sphere = model->boundingSphere.Transform(finalizedPosition, finalizedRotation, finalizedScale);

		return frustrum.IsSphereVisible(sphere.offset, sphere.Radius);
	};

	virtual bool isVisible()
	{

		if (model == nullptr)
			return false;

		return Visible;
	}

	bool IsCameraVisible()
	{

		if (model == nullptr)
			return false;

		return IsInFrustrum(Camera::frustum) && isVisible();
	}
	bool IsShadowVisible()
	{

		if (model == nullptr)
			return false;

		return (isVisible() && CastShadows) || CastHiddenShadows;
	}

	bool IsDetailShadow() { return CastDetailShadows; }

	void DrawForward(mat4x4 view, mat4x4 projection);

	void DrawDepth(mat4x4 view, mat4x4 projection);

	void DrawShadow(mat4x4 view, mat4x4 projection);

	void PreloadAssets();


private:

};