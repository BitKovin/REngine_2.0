#include "StaticMesh.h"

#include "Renderer/Renderer.h"

#include "Entity.hpp"

#include "Helpers/StringHelper.h"

LightVolPointData StaticMesh::GetLightVolData()
{

	if (OwnerEntity == nullptr || 
		model == nullptr||
		Level::Current->BspData.lightVols.size() == 0) return LightVolPointData{ vec3(0),vec3(1),vec3(0) };

	vec3 samplePos = OwnerEntity->Position + vec3(0,0.75,0);

	auto light = Level::Current->BspData.GetLightvolColor(samplePos * MAP_SCALE);

	//DebugDraw::Line(samplePos, samplePos + light.direction, 0.01f);
	return light;

}

bool StaticMesh::IsCameraVisible()
{
	if (model == nullptr)
		return false;

	if (Level::Current->BspData.m_numOfVerts)
	{

		int cameraC = Level::Current->BspData.FindClusterAtPosition(Camera::finalizedPosition);
		int targetC = Level::Current->BspData.FindClusterAtPosition(Position + model->boundingSphere.offset / 2.0f);

		if (Level::Current->BspData.IsClusterVisible(cameraC, targetC) == false)
		{
			return false;
		}
	}

	return IsInFrustrum(Camera::frustum) && isVisible();
}

void StaticMesh::DrawForward(mat4x4 view, mat4x4 projection)
{

	if (model == nullptr) return;

	if (DepthWrite)
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}

	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("default_vertex", PixelShader);

	forward_shader_program->UseProgram();

	Renderer::SetSurfaceShaderUniforms(forward_shader_program);

	mat4x4 world = finalizedWorld;

	auto lightData = GetLightVolData();

	forward_shader_program->SetUniform("light_color", lightData.ambientColor);
	forward_shader_program->SetUniform("direct_light_color", lightData.directColor);
	forward_shader_program->SetUniform("direct_light_dir", lightData.direction);

	forward_shader_program->SetUniform("view", view);
	forward_shader_program->SetUniform("projection", projection);

	forward_shader_program->SetUniform("world", world);

	forward_shader_program->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(forward_shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (ColorTexture == nullptr)
		{

			string baseTextureName;

			for (auto texture : mesh.textures)
			{
				if (texture.type == aiTextureType_BASE_COLOR)
				{
					baseTextureName = texture.src;
					break;
				}
			}



			if (mesh.cachedBaseColor == nullptr)
			{
				const string& textureRoot = TexturesLocation;

				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
			}

			if (mesh.cachedEmissiveColor == nullptr)
			{
				const string& textureRoot = TexturesLocation;

				mesh.cachedEmissiveColor = AssetRegistry::GetTextureFromFile(textureRoot + StringHelper::Replace(baseTextureName, ".", "_em."));
			}

			Texture* texture = mesh.cachedBaseColor;
			Texture* textureEm = mesh.cachedEmissiveColor;

			forward_shader_program->SetTexture("u_texture", texture);
			forward_shader_program->SetTexture("u_textureEmissive", textureEm);
		}
		else
		{
			forward_shader_program->SetTexture("u_texture", ColorTexture);
			forward_shader_program->SetTexture("u_textureEmissive", EmissiveTexture);
		}

		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}


}

void StaticMesh::DrawDepth(mat4x4 view, mat4x4 projection)
{

	bool mask = Transparent;

	ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", mask ? "mask_pixel" : "empty_pixel");

	shader_program->UseProgram();

	mat4x4 world = finalizedWorld;

	shader_program->SetUniform("view", view);
	shader_program->SetUniform("projection", projection);

	shader_program->SetUniform("world", world);

	shader_program->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{
		if (mask)
		{
			if (ColorTexture == nullptr)
			{

				string baseTextureName;

				for (auto texture : mesh.textures)
				{
					if (texture.type == aiTextureType_BASE_COLOR)
					{
						baseTextureName = texture.src;
						break;
					}
				}



				if (mesh.cachedBaseColor == nullptr)
				{
					const string textureRoot = TexturesLocation;

					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
				}

				Texture* texture = mesh.cachedBaseColor;

				shader_program->SetTexture("u_texture", texture);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}
		}
		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}
}

void StaticMesh::DrawShadow(mat4x4 view, mat4x4 projection)
{

	bool mask = Transparent;

	ShaderProgram* shader_program = ShaderManager::GetShaderProgram("default_vertex", mask ? "mask_pixel" : "empty_pixel");

	shader_program->UseProgram();

	mat4x4 world = finalizedWorld;

	shader_program->SetUniform("view", view);
	shader_program->SetUniform("projection", projection);

	shader_program->SetUniform("world", world);

	shader_program->SetUniform("isViewmodel", false);

	ApplyAdditionalShaderParams(shader_program);


	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (mask)
		{

			if (ColorTexture == nullptr)
			{

				string baseTextureName;

				for (auto texture : mesh.textures)
				{
					if (texture.type == aiTextureType_BASE_COLOR)
					{
						baseTextureName = texture.src;
						break;
					}
				}



				if (mesh.cachedBaseColor == nullptr)
				{
					const string& textureRoot = TexturesLocation;

					mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
				}

				Texture* texture = mesh.cachedBaseColor;

				shader_program->SetTexture("u_texture", texture);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
			}
		}
		mesh.VAO->Bind();

		if (mesh.VAO->IsInstanced())
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0, mesh.VAO->GetInstanceCount());
		}
		else if (numInstances < 0)
		{
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.VAO->IndexCount), GL_UNSIGNED_INT, 0);
		}

		VertexArrayObject::Unbind();

	}
}

void StaticMesh::PreloadAssets()
{

	for (roj::SkinnedMesh& mesh : model->meshes)
	{

		if (ColorTexture == nullptr)
		{

			string baseTextureName;

			for (auto texture : mesh.textures)
			{
				if (texture.type == aiTextureType_BASE_COLOR)
				{
					baseTextureName = texture.src;
					break;
				}
			}

			if (mesh.cachedBaseColor == nullptr)
			{
				const string textureRoot = TexturesLocation;

				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
			}

		}

	}
}
