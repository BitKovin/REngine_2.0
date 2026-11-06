#include "UiGpu.h"
#include <chrono>

#include <BgfxStateManager.h>
#include <Camera.h>
#include <Logger.hpp>
#include <Shader.hpp>
#include <ShaderManager.h>
#include <Time.hpp>
#include <UI/UiManager.h>
#include <UI/UiRenderer.h>
#include <UI/Render/UiGradientRamps.h>

#include <cstring>

namespace
{
    UiPrimBuffer s_prims;
    bgfx::VertexLayout s_layout;
    std::vector<Shader*> s_programs;
    bgfx::TextureHandle s_white = BGFX_INVALID_HANDLE;
    bool s_initialised = false;

    uint64_t s_statePremul = 0;
    uint64_t s_stateAlpha = 0;
    uint64_t s_stateText = 0;
    uint64_t s_stateBlend[4] = {};
    uint8_t s_programShadow = 0;

    UiGpu::Stats s_running = {};
    UiGpu::Stats s_lastFrame = {};
    bool s_loggedTransientFull = false;
    bool s_loggedBadProgram = false;
}

namespace UiGpu
{
    void Init()
    {
        if (s_initialised)
            return;

        s_layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord2, 2, bgfx::AttribType::Float)
            .end();

        s_programs.clear();
        Shader* base = ShaderManager::GetShaderProgram("ui2/vs_ui2", "ui2/fs_ui2");
        if (base == nullptr || !base->IsValid())
            Logger::Error("[UiRenderer] ui2/vs_ui2 + ui2/fs_ui2 failed to load; new UI drawing disabled");
        s_programs.push_back(base);

        const uint8_t white[4] = { 255, 255, 255, 255 };
        s_white = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(white, 4));

        s_prims.Init();
        UiGradientRamps::Init();
        s_programShadow = RegisterProgram("ui2/vs_ui2", "ui2/fs_ui2_shadow");

        // Legacy states, computed once exactly as the legacy code computes them.
        const uint64_t saved = BgfxStateManager::GetState();
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
        s_statePremul = BgfxStateManager::GetState() | BGFX_STATE_MSAA;      // Apply() adds MSAA
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
        s_stateAlpha = BgfxStateManager::GetState() | BGFX_STATE_MSAA;
        BgfxStateManager::SetState(saved);
        {
            const uint64_t common = s_statePremul & ~BGFX_STATE_BLEND_MASK;
            s_stateBlend[0] = s_statePremul;
            s_stateBlend[1] = common | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);                       // plus-lighter
            s_stateBlend[2] = common | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_INV_SRC_ALPHA);        // multiply
            s_stateBlend[3] = common | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_COLOR);             // screen
        }
        s_stateText = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS |
                      BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

        s_running = {};
        s_lastFrame = {};
        s_initialised = true;
    }

    void Shutdown()
    {
        if (!s_initialised)
            return;
        UiGradientRamps::Shutdown();
        s_prims.Shutdown();
        if (bgfx::isValid(s_white))
        {
            bgfx::destroy(s_white);
            s_white = BGFX_INVALID_HANDLE;
        }
        s_programs.clear();
        s_initialised = false;
    }

    UiPrimBuffer& Prims() { return s_prims; }
    bgfx::TextureHandle WhiteTexture() { return s_white; }

    uint8_t RegisterProgram(const char* vs, const char* fs)
    {
        Shader* shader = ShaderManager::GetShaderProgram(vs, fs);
        if (shader == nullptr || !shader->IsValid() || s_programs.size() >= 255)
        {
            Logger::Error("[UiRenderer] UI program %s + %s is not available", vs, fs);
            return 0;
        }
        s_programs.push_back(shader);
        return static_cast<uint8_t>(s_programs.size() - 1);
    }

    uint64_t StateLegacyPremultiplied() { return s_statePremul; }
    uint64_t StateLegacyAlpha() { return s_stateAlpha; }
    uint64_t StateLegacyText() { return s_stateText; }
    uint64_t StateForBlend(int blendMode) { return s_stateBlend[blendMode >= 0 && blendMode < 4 ? blendMode : 0]; }
    uint8_t ProgramShadow() { return s_programShadow; }

    Projection CurrentProjection()
    {
        Projection projection;
        projection.Height = static_cast<float>(UiManager::GetScaledUiHeight());
        projection.Width = projection.Height * Camera::AspectRatio;
        if (UiRenderer::customViewport)
        {
            projection.Width = static_cast<float>(UiRenderer::customViewportSize.x);
            projection.Height = static_cast<float>(UiRenderer::customViewportSize.y);
        }
        return projection;
    }

    void Submit(UiDrawList& list, uint16_t viewId, const Projection& projection)
    {
        if (!s_initialised)
        {
            list.Clear();
            return;
        }
        CpuTimer timer(kCpuSubmit);

        s_prims.UploadPending();
        UiGradientRamps::UploadPending();

        const auto& items = list.Items();
        const uint32_t totalVertices = list.TotalVertices();
        const uint32_t totalIndices = list.TotalIndices();
        std::vector<uint32_t> firstVertex(items.size(), 0), firstIndex(items.size(), 0);

        bgfx::TransientVertexBuffer tvb = {};
        bgfx::TransientIndexBuffer tib = {};
        bool geometryOk = false;
        if (totalVertices > 0 && totalIndices > 0)
        {
            if (bgfx::getAvailTransientVertexBuffer(totalVertices, s_layout) >= totalVertices &&
                bgfx::getAvailTransientIndexBuffer(totalIndices) >= totalIndices)
            {
                bgfx::allocTransientVertexBuffer(&tvb, totalVertices, s_layout);
                bgfx::allocTransientIndexBuffer(&tib, totalIndices);
                // Items own their vertices (later geometry may have joined an earlier item), so they are packed here.
                uint32_t v = 0, i = 0;
                for (size_t n = 0; n < items.size(); ++n)
                {
                    if (items[n].External)
                        continue;
                    firstVertex[n] = v;
                    firstIndex[n] = i;
                    std::memcpy(static_cast<UiRender::Vertex*>(static_cast<void*>(tvb.data)) + v, items[n].Verts.data(),
                                items[n].Verts.size() * sizeof(UiRender::Vertex));
                    std::memcpy(static_cast<uint16_t*>(static_cast<void*>(tib.data)) + i, items[n].Indices.data(),
                                items[n].Indices.size() * sizeof(uint16_t));
                    v += static_cast<uint32_t>(items[n].Verts.size());
                    i += static_cast<uint32_t>(items[n].Indices.size());
                }
                geometryOk = true;
            }
            else if (!s_loggedTransientFull)
            {
                Logger::Error("[UiRenderer] transient buffers are full; UI geometry skipped this frame");
                s_loggedTransientFull = true;
            }
        }

        const glm::mat4 proj = glm::ortho(0.f, projection.Width, projection.Height, 0.f, -1.f, 1.f);

        uint32_t geometryDraws = 0;
        uint32_t externals = 0;
        for (size_t n = 0; n < items.size(); ++n)
        {
            const UiDrawList::Item& item = items[n];
            if (item.External)
            {
                list.Externals()[item.ExternalIndex](viewId);
                ++externals;
                continue;
            }
            if (!geometryOk)
                continue;

            Shader* shader = item.Key.Program < s_programs.size() ? s_programs[item.Key.Program] : nullptr;
            if (shader == nullptr || !shader->IsValid())
            {
                if (!s_loggedBadProgram)
                {
                    Logger::Error("[UiRenderer] UI geometry uses program %d which is not valid; skipped", static_cast<int>(item.Key.Program));
                    s_loggedBadProgram = true;
                }
                continue;
            }

            shader->UseProgram();
            shader->SetUniform("u_uiProjection", proj);
            shader->SetUniform("u_uiParams", glm::vec4(static_cast<float>(UiRender::kDataWidth), static_cast<float>(UiRender::kPrimTexels),
                                                      static_cast<float>(Time::GameTimeNoPause), 0.f));
            shader->SetTexture("s_uiData", s_prims.Texture());
            shader->SetTexture("s_uiRamp", UiGradientRamps::Texture());
            bgfx::TextureHandle tex = { item.Key.TextureIdx };
            shader->SetTexture("s_uiTex0", bgfx::isValid(tex) ? tex : s_white);
            bgfx::setState(item.Key.State);
            if (item.Key.StencilRef > 0)
                bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(item.Key.StencilRef) | BGFX_STENCIL_FUNC_RMASK(0xFF) |
                                 BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
            bgfx::setVertexBuffer(0, &tvb, firstVertex[n], static_cast<uint32_t>(item.Verts.size()));
            bgfx::setIndexBuffer(&tib, firstIndex[n], static_cast<uint32_t>(item.Indices.size()));
            shader->Submit(viewId);
            ++geometryDraws;
        }

        s_running.Passes += 1;
        s_running.GeometryDraws += geometryDraws;
        s_running.Externals += externals;
        s_running.Vertices += totalVertices;
        s_running.Merged += list.MergedCount();
        const uint32_t* breaks = list.BreakCounts();
        for (int i = 0; i < static_cast<int>(UiDrawList::BreakReason::Count); ++i)
            s_running.Breaks[i] += breaks[i];

        list.Clear();
    }

    const Stats& LastFrameStats() { return s_lastFrame; }

    namespace
    {
        long long NowUs() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
    }
    CpuTimer::CpuTimer(CpuSlot slot) : m_slot(slot), m_start(NowUs()) {}
    CpuTimer::~CpuTimer() { s_running.CpuUs[m_slot] += static_cast<uint32_t>(NowUs() - m_start); }

    void OnBeforeBgfxFrame()
    {
        s_lastFrame = s_running;
        s_lastFrame.Prims = static_cast<uint32_t>(s_prims.Count());
        s_running = {};
        s_loggedTransientFull = false;
        s_loggedBadProgram = false;
        s_prims.ResetFrame();
        UiGradientRamps::OnBeforeBgfxFrame();
    }
}
