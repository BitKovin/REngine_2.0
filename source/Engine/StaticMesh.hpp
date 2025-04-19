#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

#include "skinned_model.hpp"

#include "glm.h"

using namespace std;

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

public:

	roj::SkinnedModel* model = nullptr;

	

	Texture* ColorTexture = nullptr;

	vec3 Position = vec3(0);
	vec3 Rotation = vec3(0);

	vec3 Scale = vec3(1);

	string TexturesLocation = "GameData/Textures/";

	bool DepthWrite = true;

	StaticMesh()
	{

	}
	~StaticMesh()
	{

	}

	void SetPixelShader(string name)
	{
		PixelShader = name;

		forward_shader_program = nullptr;

	}

	mat4 GetWorldMatrix()
	{
		return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale);
	}

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

		auto sphere = model->boundingSphere.Transform(finalizedPosition, finalizedRotation, finalizedScale);

		return frustrum.IsSphereVisible(sphere.offset, sphere.Radius);
	};
	


	void DrawForward(mat4x4 view, mat4x4 projection);

	void DrawDepth(mat4x4 view, mat4x4 projection)
	{
		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", "empty_pixel");

		shader_program->UseProgram();

		mat4x4 world = finalizedWorld;

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		shader_program->SetUniform("isViewmodel", IsViewmodel);

		ApplyAdditionalShaderParams(shader_program);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{
			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}
	}

	void DrawShadow(mat4x4 view, mat4x4 projection)
	{
		ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", "empty_pixel");

		shader_program->UseProgram();

		mat4x4 world = finalizedWorld;

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);

		ApplyAdditionalShaderParams(shader_program);


		for (const roj::SkinnedMesh& mesh : model->meshes)
		{
			mesh.VAO->Bind();
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}
	}

	void PreloadAssets();


private:

};