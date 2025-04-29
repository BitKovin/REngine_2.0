#include "RibbonEmitter.h"
#include "../MathHelper.hpp"

#include "../Renderer/Renderer.h"

RibbonEmitter::RibbonEmitter()
    : decl(VertexData::Declaration())
{
}

RibbonEmitter::~RibbonEmitter() 
{
    if (vb)
    {
        delete vb;
    }
    
    if (ib)
    {
        delete ib;
    }
    
}

void RibbonEmitter::RenderRibbon(const std::vector<Particle>& inParticles) {
    primitiveCount = 0;

    if (inParticles.size() < 2 || destroyed)
        return;

    // 1) copy + append emitters
    std::vector<Particle> particles = inParticles;
    if (Emitting) {
        Particle np = GetNewParticle();
        np.position = Position + MathHelper::GetForwardVector(particles.back().globalRotation) * 0.001f;
        particles.push_back(np);

        np = GetNewParticle();
        np.position = Position + MathHelper::GetForwardVector(particles.back().globalRotation) * 0.001f;
        particles.push_back(np);
    }

    int n = int(particles.size());
    int vCount = n * 2;
    int idxCount = (n - 1) * 6;

    // 2) resize CPU arrays
    verts.resize(vCount);
    idxs.resize(idxCount);
    GenerateIndices(idxs, n);

    // 3) fill vertex data
    glm::vec3 camPos = Camera::position;
    for (int i = 0; i < n; ++i) {
        const Particle& p = particles[i];
        glm::vec3 P = p.position;

        glm::vec3 dir = (i < n - 1)
            ? glm::normalize(P - particles[i + 1].position)
            : glm::normalize(particles[i - 1].position - P);

        glm::vec3 camFwd = glm::normalize(P - camPos);
        glm::vec3 perp = glm::normalize(glm::cross(dir, camFwd));

        float half = p.Size * 0.5f;
        int   b = i * 2;

        verts[b + 0].Position = P + perp * half;
        verts[b + 0].TextureCoordinate = glm::vec2(float(i) / (n - 1), 0.0f);
        verts[b + 0].Color = p.Color;

        verts[b + 1].Position = P - perp * half;
        verts[b + 1].TextureCoordinate = glm::vec2(float(i) / (n - 1), 1.0f);
        verts[b + 1].Color = p.Color;
    }

    primitiveCount = idxCount / 3;

    // 4) (re)allocate GPU buffers if size changed
    if (n != lastCount) 
    {
        if (vb)
        {
            delete vb; 
            vb = nullptr;
        }
        
        if (ib)
        {
            delete ib; 
            ib = nullptr;
        }
        

        vb = new VertexBuffer(verts, decl, GL_DYNAMIC_DRAW);
        ib = new IndexBuffer(idxs, GL_STATIC_DRAW);

        lastCount = n;
    }
    else {
        // update only vertex positions/colors/UVs
        vb->UpdateData(verts, 0, GL_DYNAMIC_DRAW);
        // indices never change unless count changes
    }

    // 5) draw
    vb->Bind();
    ib->Bind();

    // set up attributes from declaration
    for (auto& e : decl.GetElements()) {
        glEnableVertexAttribArray(e.index);
        bool intType = (e.type == GL_INT || e.type == GL_UNSIGNED_INT ||
            e.type == GL_SHORT || e.type == GL_UNSIGNED_SHORT ||
            e.type == GL_BYTE || e.type == GL_UNSIGNED_BYTE);
        if (intType && !e.normalized) {
            glVertexAttribIPointer(
                e.index, e.componentCount, e.type,
                e.stride, e.offset
            );
        }
        else {
            glVertexAttribPointer(
                e.index, e.componentCount, e.type,
                e.normalized, e.stride, e.offset
            );
        }

    }

    glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, nullptr);

    // disable attribute arrays
    for (auto& e : decl.GetElements()) {
        glDisableVertexAttribArray(e.index);
    }

    VertexBuffer::Unbind();
    IndexBuffer::Unbind();
}

void RibbonEmitter::FinalizeFrameData()
{
    finalizedParticles = Particles;
}

void RibbonEmitter::DrawForward(mat4x4 view, mat4x4 projection)
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
        forward_shader_program = ShaderManager::GetShaderProgram();

    forward_shader_program->UseProgram();

    forward_shader_program->SetUniform("view", view);
    forward_shader_program->SetUniform("projection", projection);

    forward_shader_program->SetUniform("is_particle", true);
    forward_shader_program->SetUniform("is_decal", false);

    forward_shader_program->SetUniform("isViewmodel", false);

    Renderer::SetSurfaceShaderUniforms(forward_shader_program);

    forward_shader_program->SetTexture("u_texture", savedTexture);

    RenderRibbon(finalizedParticles);

    forward_shader_program->SetUniform("is_particle", false);
    forward_shader_program->SetUniform("is_decal", false);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void RibbonEmitter::GenerateIndices(std::vector<GLuint>& dst, int n) {
    for (int i = 1; i < n; ++i) {
        int io = (i - 1) * 6;
        int vo = i * 2;
        dst[io + 0] = GLuint(vo);
        dst[io + 1] = GLuint(vo - 1);
        dst[io + 2] = GLuint(vo - 2);
        dst[io + 3] = GLuint(vo);
        dst[io + 4] = GLuint(vo + 1);
        dst[io + 5] = GLuint(vo - 1);
    }
}
