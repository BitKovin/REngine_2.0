#pragma once

#include "GoreTypes.h"

#include <DrawCommands/StaticMeshDrawCommand.h>
#include <glm.h>

#include <array>

class Texture;

namespace Gore
{
    // Everything the gore shaders need for one body, computed in GoreSkeletalMesh::PreFinalize.
    // Meaning of every array: GORE_IMPLEMENTATION.md P2.4.
    struct GpuData
    {
        glm::vec4 counts = glm::vec4(0);       // x wounds, y cuts, z splats, w decals
        glm::vec4 params = glm::vec4(0);       // x homogeneousDepth (0/1), y isGib (0/1), z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
        glm::vec4 bloodColor = glm::vec4(0.35f, 0.02f, 0.02f, 1.0f);
        std::array<glm::vec4, kMaxWounds * 3> woundBind{}, woundWorld{};
        std::array<glm::vec4, kMaxWounds> woundParams{};   // x roughness, y rimWidth, z healScale, w seed
        std::array<glm::vec4, kMaxWounds> woundExtra{};    // x reach (world m), y mean radius (world m), z kind, w 0
        std::array<glm::vec4, kMaxCuts> cutBind{}, cutBindSphere{}, cutRegrowBind{}, cutParams{};
        std::array<glm::vec4, kMaxCuts> cutWorld{}, cutWorldSphere{}, cutRegrowWorld{}, cutCenter{};
        std::array<glm::vec4, kMaxCuts * 2> cutFrame{};
        std::array<glm::vec4, kMaxSplats> splats{};
        std::array<glm::vec4, kMaxDecals> decals{};        // xyz bind position, w ring radius
        Texture* flesh = nullptr;
        Texture* fleshNormal = nullptr;
        Texture* edgeNoise = nullptr;
        Texture* blood = nullptr;
    };

    class GoreDrawCommand : public SkeletalMeshDrawCommand
    {
    public:
        enum class Pass : uint8_t { Front, Interior, Cavity };

        Pass pass = Pass::Front;
        bool goreActive = false;                // false: behaves exactly like SkeletalMeshDrawCommand
        bool shell = false;                     // submesh role Shell (interior draws only the dark back side)
        const GpuData* gore = nullptr;          // points at GoreSkeletalMesh::finalGpu (changes only in FinalizeFrameData)

        Shader* GetForwardShader() const override;
        Shader* GetDepthShader() const override;
        Shader* GetCustomIdShader() const override;
        void GetShadowVolumeShaders(Shader*& cap, Shader*& edge) const override;
        void ApplyAdditionalShaderParams(Shader* shader) override;

        void DrawForward(const mat4& view, const mat4& projection) override;
        void DrawDepth(const mat4& view, const mat4& projection) override;
        void DrawCustomId(const mat4& view, const mat4& projection) override;

        // Copies everything the interior command shares with its front command (buffers, transform, light, material,
        // palette pointer, flags). Field by field - never operator= (it would copy the range vectors' storage).
        void CopyCommonFrom(const GoreDrawCommand& front);

    private:
        mat4 viewProj = mat4(1.0f);   // set by the Draw* overrides before calling the base
        vec3 cameraPos = vec3(0.0f);

        void CaptureCamera(const mat4& view, const mat4& projection);
    };
}
