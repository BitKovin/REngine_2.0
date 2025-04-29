#pragma once

#include "ParticleEmitter.h"
#include "../VertexData.h"
#include "../Camera.h"
#include "../glm.h"
#include <vector>

class RibbonEmitter : public ParticleEmitter {
public:
    RibbonEmitter();
    ~RibbonEmitter();

    /// Call once per frame, after FinalizeFrameData();
    /// Assumes your ribbon shader is already bound and its view/proj uniforms set.
    void RenderRibbon(const std::vector<Particle>& particles);

    int GetPrimitiveCount() const { return primitiveCount; }

    void FinalizeFrameData();

    void DrawForward(mat4x4 view, mat4x4 projection);

private:
    void GenerateIndices(std::vector<GLuint>& dst, int count);

    // CPU‐side buffers
    std::vector<VertexData> verts;
    std::vector<GLuint>     idxs;

    // GL helpers
    VertexDeclaration decl;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;

    int lastCount = 0;
    int primitiveCount = 0;
};
