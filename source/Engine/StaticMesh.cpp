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
			if (mesh.cachedBaseColor == nullptr)
			{
				string baseTextureName;

				for (auto texture : mesh.textures)
				{
					Logger::Log("t: " + texture.src);
					if (texture.type == aiTextureType_BASE_COLOR)
					{
						baseTextureName = texture.src;
						break;
					}

				}



				const string textureRoot = TexturesLocation;
				Logger::Log(textureRoot + baseTextureName);
				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);

				Logger::Log(textureRoot + baseTextureName);

			}

			Texture* texture = mesh.cachedBaseColor;

			forward_shader_program->SetTexture("u_texture", texture);
		}
		else
		{
			forward_shader_program->SetTexture("u_texture", ColorTexture);
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

				if (mesh.cachedEmissiveColor == nullptr)
				{
					const string& textureRoot = TexturesLocation;

					mesh.cachedEmissiveColor = AssetRegistry::GetTextureFromFile(textureRoot + StringHelper::Replace(baseTextureName, ".", "_em."));
				}

				Texture* texture = mesh.cachedBaseColor;
				Texture* textureEm = mesh.cachedEmissiveColor;

				shader_program->SetTexture("u_texture", texture);
				shader_program->SetTexture("u_textureEmissive", textureEm);
			}
			else
			{
				shader_program->SetTexture("u_texture", ColorTexture);
				shader_program->SetTexture("u_textureEmissive", EmissiveTexture);
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
