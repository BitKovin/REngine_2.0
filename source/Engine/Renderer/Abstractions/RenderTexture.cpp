// RenderTexture.cpp
#include "RenderTexture.h"
#include "../../gl.h"
#include <stdexcept>

//------------------------------------------------------------------------------
// On WebGL (Emscripten) we must fall back to single‐sample 2D only
#if defined(__EMSCRIPTEN__)
#define DISABLE_MULTISAMPLE
#endif

RenderTexture::RenderTexture(uint32_t width, uint32_t height,
    TextureFormat format,
    TextureType type,
    GLenum minFilter,
    GLenum magFilter,
    GLenum wrap,
    uint32_t samples)
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_type(type)
    , m_samples(samples)
{
#ifndef DISABLE_MULTISAMPLE
    validateSampleCount();
#else
    // force single‐sample on WebGL
    m_samples = 1;
    // if someone passed Texture2DMultisample, silently treat as Texture2D
    if (m_type == TextureType::Texture2DMultisample)
        m_type = TextureType::Texture2D;
#endif

    glGenTextures(1, &m_id);
    allocateStorage();
    setParameters(minFilter, magFilter, wrap);
}

RenderTexture::~RenderTexture() {
    glDeleteTextures(1, &m_id);
}

void RenderTexture::bind(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(static_cast<GLenum>(m_type), m_id);
}

void RenderTexture::unbind() const {
    glBindTexture(static_cast<GLenum>(m_type), 0);
}

void RenderTexture::resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return;
    m_width = width;
    m_height = height;
    allocateStorage();
}

void RenderTexture::setSamples(uint32_t samples) {
    if (samples == m_samples) return;
    m_samples = samples;
#ifndef DISABLE_MULTISAMPLE
    validateSampleCount();
    allocateStorage();
#else
    // no-op under DISABLE_MULTISAMPLE
#endif
}

void RenderTexture::allocateStorage() {
    GLenum target = static_cast<GLenum>(m_type);
    auto [internalFmt, baseFmt, dataType] = getFormatInfo();

    glBindTexture(target, m_id);

#if !defined(DISABLE_MULTISAMPLE)
    if (m_type == TextureType::Texture2DMultisample) {
        glTexImage2DMultisample(
            GL_TEXTURE_2D_MULTISAMPLE,
            m_samples,
            internalFmt,
            m_width, m_height,
            GL_TRUE
        );
        glBindTexture(target, 0);
        return;
    }
#endif

    // single‐sample 2D or cubemap path
    if (m_type == TextureType::Texture2D) {
        glTexImage2D(GL_TEXTURE_2D,
            0,
            internalFmt,
            m_width, m_height,
            0,
            baseFmt, dataType,
            nullptr);
    }
    else if (m_type == TextureType::Cubemap) {
        for (unsigned face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                0,
                internalFmt,
                m_width, m_height,
                0,
                baseFmt, dataType,
                nullptr);
        }
    }

    glBindTexture(target, 0);
}

void RenderTexture::setParameters(GLenum minFilter,
    GLenum magFilter,
    GLenum wrap)
{
    GLenum target = static_cast<GLenum>(m_type);
    glBindTexture(target, m_id);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    if (m_type == TextureType::Cubemap)
        glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
    glBindTexture(target, 0);
}

std::tuple<GLenum, GLenum, GLenum> RenderTexture::getFormatInfo() const {
    switch (m_format) {
    case TextureFormat::R8:            return { GL_R8,           GL_RED,            GL_UNSIGNED_BYTE };
    case TextureFormat::RG8:           return { GL_RG8,          GL_RG,             GL_UNSIGNED_BYTE };
    case TextureFormat::RGB8:          return { GL_RGB8,         GL_RGB,            GL_UNSIGNED_BYTE };
    case TextureFormat::RGBA8:         return { GL_RGBA8,        GL_RGBA,           GL_UNSIGNED_BYTE };
    case TextureFormat::R16F:          return { GL_R16F,         GL_RED,            GL_HALF_FLOAT };
    case TextureFormat::RG16F:         return { GL_RG16F,        GL_RG,             GL_HALF_FLOAT };
    case TextureFormat::RGB16F:        return { GL_RGB16F,       GL_RGB,            GL_HALF_FLOAT };
    case TextureFormat::RGBA16F:       return { GL_RGBA16F,      GL_RGBA,           GL_HALF_FLOAT };
    case TextureFormat::R32F:          return { GL_R32F,         GL_RED,            GL_FLOAT };
    case TextureFormat::RG32F:         return { GL_RG32F,        GL_RG,             GL_FLOAT };
    case TextureFormat::RGB32F:        return { GL_RGB32F,       GL_RGB,            GL_FLOAT };
    case TextureFormat::RGBA32F:       return { GL_RGBA32F,      GL_RGBA,           GL_FLOAT };
    case TextureFormat::Depth16:       return { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT };
    case TextureFormat::Depth24:       return { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
    case TextureFormat::Depth32F:      return { GL_DEPTH_COMPONENT32F,GL_DEPTH_COMPONENT, GL_FLOAT };
    case TextureFormat::Depth24Stencil8: return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
    case TextureFormat::Depth32FStencil8:
        return { GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV };
    }

    throw std::runtime_error("RenderTexture: unknown TextureFormat");
}

void RenderTexture::validateSampleCount() const {
#if defined(DISABLE_MULTISAMPLE)
    // no checks under WebGL
#else
    if (m_samples > 1 && m_type != TextureType::Texture2DMultisample) {
        throw std::invalid_argument(
            "Samples > 1 requires Texture2DMultisample type");
    }
#endif
}
