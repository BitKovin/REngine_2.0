#include "StaticMesh.h"

#include "Renderer/Renderer.h"

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
				const string textureRoot = TexturesLocation;

				mesh.cachedBaseColor = AssetRegistry::GetTextureFromFile(textureRoot + baseTextureName);
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
