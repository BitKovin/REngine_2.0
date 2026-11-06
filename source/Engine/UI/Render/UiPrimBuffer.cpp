#include "UiPrimBuffer.h"

#include <Logger.hpp>
#include <Profiling/ResourceStatistics.hpp>

using namespace UiRender;

void UiPrimBuffer::Init()
{
    const uint16_t formatCaps = bgfx::getCaps()->formats[bgfx::TextureFormat::RGBA32F];
    if ((formatCaps & BGFX_CAPS_FORMAT_TEXTURE_2D) == 0)
    {
        Logger::Error("[UiRenderer] RGBA32F textures unsupported; new UI drawing disabled");
        return;
    }

    m_texture = bgfx::createTexture2D(
        static_cast<uint16_t>(kDataWidth), static_cast<uint16_t>(kDataHeight), false, 1,
        bgfx::TextureFormat::RGBA32F, BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);

    if (bgfx::isValid(m_texture))
    {
        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture, m_texture.idx,
            static_cast<size_t>(kDataWidth) * kDataHeight * sizeof(glm::vec4), "UiPrimData");
    }

    m_cpu.assign(static_cast<size_t>(kDataWidth) * kDataHeight, glm::vec4(0.f));
    m_count = m_uploadedCount = 0;
    m_loggedFull = false;
}

void UiPrimBuffer::Shutdown()
{
    if (bgfx::isValid(m_texture))
    {
        ResourceStatistics::Instance().unregisterResource(ResourceType::Texture, m_texture.idx);
        bgfx::destroy(m_texture);
        m_texture = BGFX_INVALID_HANDLE;
    }
    m_cpu.clear();
    m_cpu.shrink_to_fit();
    m_count = m_uploadedCount = 0;
}

int UiPrimBuffer::Add(const PrimBlock& block)
{
    if (!bgfx::isValid(m_texture))
        return -1;
    if (m_count >= kMaxPrims)
    {
        if (!m_loggedFull)
        {
            Logger::Error("[UiRenderer] primitive buffer full (%d primitives this frame); the rest is not drawn", kMaxPrims);
            m_loggedFull = true;
        }
        return -1;
    }
    for (int i = 0; i < kPrimTexels; ++i)
        m_cpu[static_cast<size_t>(m_count) * kPrimTexels + i] = block.T[i];
    return m_count++;
}

void UiPrimBuffer::UploadPending()
{
    if (!bgfx::isValid(m_texture) || m_uploadedCount == m_count)
        return;
    const int firstRow = (m_uploadedCount * kPrimTexels) / kDataWidth;
    const int lastRow = ((m_count * kPrimTexels) - 1) / kDataWidth;
    const int rows = lastRow - firstRow + 1;
    bgfx::updateTexture2D(
        m_texture, 0, 0, 0, static_cast<uint16_t>(firstRow),
        static_cast<uint16_t>(kDataWidth), static_cast<uint16_t>(rows),
        bgfx::copy(&m_cpu[static_cast<size_t>(firstRow) * kDataWidth], static_cast<uint32_t>(rows) * kDataWidth * sizeof(glm::vec4)));
    m_uploadedCount = m_count;
}

void UiPrimBuffer::ResetFrame()
{
    m_count = m_uploadedCount = 0;
    m_loggedFull = false;
}
