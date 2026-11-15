#pragma once

#include "GoreTypes.h"

#include <DrawCommands/StaticMeshDrawCommand.h>
#include <glm.h>

#include <array>

class Texture;

namespace Gore
{
    // Everything the gore shaders need for one body, computed in GoreSkeletalMesh::PreFinalize.
    struct GpuData
    {
        glm::vec4 counts = glm::vec4(0);       // x wounds, y cuts, z splats
        glm::vec4 params = glm::vec4(0);       // y isGib (0/1), z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
        glm::vec4 bloodColor = glm::vec4(0.35f, 0.02f, 0.02f, 1.0f);
        std::array<glm::vec4, kMaxWounds * 3> woundBind{};
        std::array<glm::vec4, kMaxWounds> woundParams{};   // x roughness, y rimWidth, z healScale, w seed
        std::array<glm::vec4, kMaxWounds> woundExtra{};    // x taper (0 when a cavity closes the hole), y anatomy stays, z layers share the hole edge
        std::array<glm::vec4, kMaxCuts> cutBind{}, cutBindSphere{}, cutRegrowBind{}, cutParams{};
        std::array<glm::vec4, kMaxSplats> splats{};        // xyz centre, w extent
        std::array<glm::vec4, kMaxSplats> splatLook{};     // xyz direction (zero = a plain soft soak), w 1 = a bullet hole picture, 0 = a splash
        Texture* flesh = nullptr;
        Texture* edgeNoise = nullptr;
        Texture* blood = nullptr;
        Texture* decal = nullptr;
        Texture* bone = nullptr;
        Texture* guts = nullptr;
    };

    class GoreDrawCommand : public SkeletalMeshDrawCommand
    {
    public:
        enum class Pass : uint8_t { Front, Shell, Anatomy };

        Pass pass = Pass::Front;
        bool goreActive = false;                // false: behaves exactly like SkeletalMeshDrawCommand
        const GpuData* gore = nullptr;          // points at GoreSkeletalMesh::finalGpu (changes only in FinalizeFrameData)
        float layer = 0.0f;                     // 0 skin, 1..kShellLayers shells, kAnatomyLayer anatomy
        float insetFraction = 0.0f;             // shells: share of the max inset
        bool cap = false;                       // the flat face of a blade cut (pass Anatomy)
        bool cavity = false;                    // the closing surface of the wounds (pass Anatomy, GoreCavity.h)

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
