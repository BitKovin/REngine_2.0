#include "ParticleEmitter.h"

#include "../MathHelper.hpp"

#include "../ShaderManager.h"

#include "../FrustrumCull.hpp"
#include "../BoundingSphere.hpp"

#include "../Renderer/Renderer.h"

VertexArrayObject* ParticleEmitter::bilboardVAO = nullptr;

mat4 GetWorldMatrix(const Particle& particle)
{
    return translate(particle.position) * MathHelper::GetRotationMatrix(particle.globalRotation) * scale(vec3(particle.Size));
}

void ParticleEmitter::DrawForward(mat4x4 view, mat4x4 projection)
{

    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

	glDepthMask(GL_FALSE);
	
    glDisable(GL_CULL_FACE);

	ShaderProgram* forward_shader_program = nullptr;

	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("instanced_bilboard_vertex");

	forward_shader_program->UseProgram();

    forward_shader_program->SetUniform("view", view);
    forward_shader_program->SetUniform("projection", projection);

    forward_shader_program->SetUniform("is_particle", isDecal == false);
    forward_shader_program->SetUniform("is_decal", isDecal);
    forward_shader_program->SetUniform("isViewmodel", false);

    Renderer::SetSurfaceShaderUniforms(forward_shader_program);

    forward_shader_program->SetTexture("u_texture", savedTexture);

    SetInstanceData(instances);

	bilboardVAO->Bind();
	
    int instanceCount = static_cast<int>(instances.size());
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(bilboardVAO->IndexCount), GL_UNSIGNED_INT, 0, instanceCount);

    forward_shader_program->SetUniform("is_particle", false);
    forward_shader_program->SetUniform("is_decal", false);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void ParticleEmitter::FinalizeFrameData()
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    finalizedParticles = Particles;

    // Use finalized camera data for consistency
    vec3 cameraPosition = Camera::finalizedPosition;
    vec3 cameraRotation = Camera::finalizedRotation;

    // Compute camera orientation vectors
    vec3 cameraForward = MathHelper::GetForwardVector(cameraRotation);
    vec3 cameraRight = MathHelper::GetRightVector(cameraRotation);
    vec3 cameraUp = MathHelper::GetUpVector(cameraRotation);

    if (DepthSorting) 
    {

        // Sort particles back-to-front (farthest first)
        std::sort(finalizedParticles.begin(), finalizedParticles.end(),
            [cameraForward, cameraPosition](const Particle& a, const Particle& b) {
                float depthA = glm::dot(cameraForward, a.position - cameraPosition);
                float depthB = glm::dot(cameraForward, b.position - cameraPosition);
                return depthA > depthB; // Farther particles first
            });
    }
    // Prepare instance data
    instances.clear();
    instances.reserve(finalizedParticles.size());

    for (const auto& particle : finalizedParticles) {
        // Optional: Frustum culling to skip off-screen particles
        if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
            continue;

        mat4x4 world;
        if (particle.UseWorldRotation) 
        {
            // Placeholder: Implement custom world matrix if needed
            world = GetWorldMatrix(particle);
        }
        else 
        {
            // Billboard matrix ensures the particle faces the camera
            world = MathHelper::CreateBillboardMatrix(
                particle.position,
                cameraPosition,
                cameraForward,
                cameraRight,
                cameraUp,
                vec3(particle.Size),
                particle.rotation
            );
        }

        InstanceData data;
        data.ModelMatrix = world;
        data.Color = particle.Color;
        data.Color.a *= particle.Transparency;
        instances.push_back(data);
    }
}

void ParticleEmitter::InitBilboardVaoIfNeeded()
{

    if (bilboardVAO) return;

    // Define billboard vertices (a quad centered at the origin with size 1x1 meter)
// Using VertexData to leverage your declarations. Other fields are set to default.
    std::vector<VertexData> vertices(4);

    // Bottom-left
    vertices[0].Position = glm::vec3(-0.5f, -0.5f, 0.0f);
    vertices[0].TextureCoordinate = glm::vec2(0.0f, 1.0f);

    // Bottom-right
    vertices[1].Position = glm::vec3(0.5f, -0.5f, 0.0f);
    vertices[1].TextureCoordinate = glm::vec2(1.0f, 1.0f);

    // Top-right
    vertices[2].Position = glm::vec3(0.5f, 0.5f, 0.0f);
    vertices[2].TextureCoordinate = glm::vec2(1.0f, 0.0f);

    // Top-left
    vertices[3].Position = glm::vec3(-0.5f, 0.5f, 0.0f);
    vertices[3].TextureCoordinate = glm::vec2(0.0f, 0.0f);

    // Define indices for two triangles (quad)
    std::vector<GLuint> indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    std::vector<InstanceData> instanceData{ }; //empty by default

    // Create the vertex buffer for the billboard vertices
    VertexBuffer* vb = new VertexBuffer(vertices, VertexData::Declaration(), GL_STATIC_DRAW);

    // Create the index buffer for the quad
    IndexBuffer* ib = new IndexBuffer(indices, GL_STATIC_DRAW);

    // Create the instance buffer from the provided instance data
    VertexBuffer* instanceBuffer = nullptr;
    
    instanceBuffer = new VertexBuffer(instanceData, InstanceData::Declaration(), GL_STATIC_DRAW);
    

    // Create the VAO using the vertex buffer, index buffer, and instance buffer (if any)
    bilboardVAO = new VertexArrayObject(*vb, *ib, instanceBuffer);

    VertexArrayObject::Unbind();
    IndexBuffer::Unbind();
    VertexBuffer::Unbind();

}

void ParticleEmitter::SetInstanceData(std::vector<InstanceData>& instanceData)
{
    if (bilboardVAO == nullptr)
    {
        Logger::Log("null bilboard vao");
        return;
    }

    if (bilboardVAO->instanceBuffer == nullptr)
    {
        Logger::Log("null bilboard vao instanceBuffer");
        return;
    }

    bilboardVAO->instanceBuffer->UpdateData(instanceData);

}
