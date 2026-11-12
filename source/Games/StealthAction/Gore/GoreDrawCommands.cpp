#include "GoreDrawCommands.h"

#include <ShaderManager.h>
#include <Shader.hpp>
#include <Texture.hpp>

namespace Gore
{
    namespace
    {
        const char* kVertex = "gore/vs_gore";
    }

    Shader* GoreDrawCommand::GetForwardShader() const
    {
        if (!goreActive) return SkeletalMeshDrawCommand::GetForwardShader();
        return ShaderManager::GetShaderProgram(kVertex, pass == Pass::Front ? "gore/fs_gore" : "gore/fs_gore_interior");
    }

    Shader* GoreDrawCommand::GetDepthShader() const
    {
        if (!goreActive) return SkeletalMeshDrawCommand::GetDepthShader();
        return ShaderManager::GetShaderProgram(kVertex, pass == Pass::Front ? "gore/fs_gore_depth" : "gore/fs_gore_interior_depth");
    }

    Shader* GoreDrawCommand::GetCustomIdShader() const
    {
        if (!goreActive || pass != Pass::Front) return SkeletalMeshDrawCommand::GetCustomIdShader();
        return ShaderManager::GetShaderProgram(kVertex, "gore/fs_gore_customId");
    }

    void GoreDrawCommand::GetShadowVolumeShaders(Shader*& cap, Shader*& edge) const
    {
        // Only a body with cuts needs its own shadow volume shaders (phase 3).
        if (goreActive && pass == Pass::Front && gore && gore->counts.y > 0.5f)
        {
            cap = ShaderManager::GetShaderProgram("gore/vs_shadowvolcap_gore", "shadowVolume/fs_shadowvol");
            edge = ShaderManager::GetShaderProgram("gore/vs_shadowvoledge_gore", "shadowVolume/fs_shadowvol");
            return;
        }
        SkeletalMeshDrawCommand::GetShadowVolumeShaders(cap, edge);
    }

    void GoreDrawCommand::ApplyAdditionalShaderParams(Shader* shader)
    {
        SkeletalMeshDrawCommand::ApplyAdditionalShaderParams(shader);   // bones first

        if (!goreActive || gore == nullptr) return;

        const GpuData& g = *gore;
        const int nw = static_cast<int>(g.counts.x + 0.5f);
        const int nc = static_cast<int>(g.counts.y + 0.5f);
        const int ns = static_cast<int>(g.counts.z + 0.5f);

        shader->SetUniform("u_goreCounts", g.counts);
        shader->SetUniform("u_goreParams", g.params);
        shader->SetUniform("u_goreBloodColor", g.bloodColor);
        shader->SetUniform("u_goreViewProj", viewProj);
        shader->SetUniform("u_goreCameraPos", vec4(cameraPos, 1.0f));
        shader->SetUniform("u_goreShell", vec4(shell ? 1.0f : 0.0f));
        shader->SetUniform("u_goreAlphaTest", vec4(Material.NeedsAlphaDiscard() ? 1.0f : 0.0f));

        if (nw > 0)
        {
            shader->SetUniform("u_goreWoundBind", g.woundBind.data(), static_cast<size_t>(nw) * 3);
            shader->SetUniform("u_goreWoundWorld", g.woundWorld.data(), static_cast<size_t>(nw) * 3);
            shader->SetUniform("u_goreWoundParams", g.woundParams.data(), static_cast<size_t>(nw));
            shader->SetUniform("u_goreWoundExtra", g.woundExtra.data(), static_cast<size_t>(nw));
        }

        if (nc > 0)
        {
            const size_t c = static_cast<size_t>(nc);
            shader->SetUniform("u_goreCutBind", g.cutBind.data(), c);
            shader->SetUniform("u_goreCutBindSphere", g.cutBindSphere.data(), c);
            shader->SetUniform("u_goreCutRegrowBind", g.cutRegrowBind.data(), c);
            shader->SetUniform("u_goreCutParams", g.cutParams.data(), c);
            shader->SetUniform("u_goreCutWorld", g.cutWorld.data(), c);
            shader->SetUniform("u_goreCutWorldSphere", g.cutWorldSphere.data(), c);
            shader->SetUniform("u_goreCutRegrowWorld", g.cutRegrowWorld.data(), c);
            shader->SetUniform("u_goreCutCenter", g.cutCenter.data(), c);
            shader->SetUniform("u_goreCutFrame", g.cutFrame.data(), c * 2);
        }

        if (ns > 0)
            shader->SetUniform("u_goreSplats", g.splats.data(), static_cast<size_t>(ns));

        shader->SetTexture(hashed_string("u_goreFlesh"), g.flesh);
        shader->SetTexture(hashed_string("u_goreFleshNormal"), g.fleshNormal);
        shader->SetTexture(hashed_string("u_goreEdgeNoise"), g.edgeNoise);
        shader->SetTexture(hashed_string("u_goreBlood"), g.blood);
    }

    void GoreDrawCommand::CaptureCamera(const mat4& view, const mat4& projection)
    {
        viewProj = projection * view;
        cameraPos = vec3(glm::inverse(view)[3]);
    }

    void GoreDrawCommand::DrawForward(const mat4& view, const mat4& projection)
    {
        if (goreActive) CaptureCamera(view, projection);
        SkeletalMeshDrawCommand::DrawForward(view, projection);
    }

    void GoreDrawCommand::DrawDepth(const mat4& view, const mat4& projection)
    {
        if (goreActive) CaptureCamera(view, projection);
        SkeletalMeshDrawCommand::DrawDepth(view, projection);
    }

    void GoreDrawCommand::DrawCustomId(const mat4& view, const mat4& projection)
    {
        if (goreActive) CaptureCamera(view, projection);
        SkeletalMeshDrawCommand::DrawCustomId(view, projection);
    }

    void GoreDrawCommand::CopyCommonFrom(const GoreDrawCommand& front)
    {
        VertexBuffer = front.VertexBuffer;
        IndexBuffer = front.IndexBuffer;

        WorldMatrix = front.WorldMatrix;
        WorldBounds = front.WorldBounds;
        IsViewmodel = front.IsViewmodel;

        LightAmbientColor = front.LightAmbientColor;
        LightDirectColor = front.LightDirectColor;
        LightDirection = front.LightDirection;

        PixelShader = front.PixelShader;
        VertexShader = front.VertexShader;
        DepthWrite = front.DepthWrite;
        DepthPrePass = front.DepthPrePass;
        TwoSided = front.TwoSided;
        Brightness = front.Brightness;
        BlendMode = front.BlendMode;

        if (CustomShaderParams != front.CustomShaderParams)   // copied only when it changed (a map copy allocates)
            CustomShaderParams = front.CustomShaderParams;

        Hidden = front.Hidden;
        Material = front.Material;
        BoneMatrices = front.BoneMatrices;
        gore = front.gore;
    }
}
