// RenderTexture.cpp
#include "RenderTexture.h"
#include "../../gl.h"
#include <stdexcept>

//------------------------------------------------------------------------------
// On WebGL (Emscripten) we must fall back to single‐sample 2D only
#if defined(GL_ES_PROFILE)
#define DISABLE_MULTISAMPLE
#endif

RenderTexture::RenderTexture(uint32_t width, uint32_t height,
    TextureFormat format,
    TextureType type,bool sampleDepth,
    GLenum minFilter,
    GLenum magFilter,
    GLenum wrap,
    uint32_t samples)
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_type(type)
    , m_minFilter(minFilter), m_magFilter(magFilter), m_wrapF(wrap)
    , m_samples(samples), m_sampleDepth(sampleDepth)
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

bool RenderTexture::resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return false;
    m_width = width;
    m_height = height;
    allocateStorage();
    return true;
}

void RenderTexture::setTextureType(TextureType newType)
{

    if (newType == m_type) return;

    m_type = newType;

    glDeleteTextures(1, &m_id);

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
    setParameters(m_minFilter, m_magFilter, m_wrapF);

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
    if (m_type != TextureType::Texture2DMultisample)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    }

    if (m_type == TextureType::Cubemap)
        glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);

    if (m_sampleDepth)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }



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

void RenderTexture::copyFrom(const RenderTexture* src) 
{
    if (!src) {
        throw std::runtime_error("RenderTexture::copyFrom: source pointer is null");
    }

    if (m_width != src->width() || m_height != src->height() ||
        m_type != src->type() || m_format != src->format() ||
        m_samples != src->samples()) {
        throw std::runtime_error(
            "RenderTexture::copyFrom: source and destination must have same dimensions, type, format, and sample count");
    }

    GLuint fboRead = 0, fboDraw = 0;
    glGenFramebuffers(1, &fboRead);
    glGenFramebuffers(1, &fboDraw);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw);

    auto [internalFmt, baseFmt, dataType] = getFormatInfo();  // Same for src and this

    GLenum attachment;
    GLbitfield mask;
    if (baseFmt == GL_DEPTH_STENCIL) {
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    }
    else if (baseFmt == GL_DEPTH_COMPONENT) {
        attachment = GL_DEPTH_ATTACHMENT;
        mask = GL_DEPTH_BUFFER_BIT;
    }
    else {
        attachment = GL_COLOR_ATTACHMENT0;
        mask = GL_COLOR_BUFFER_BIT;
    }

    GLenum filter = (mask & GL_COLOR_BUFFER_BIT) ? GL_LINEAR : GL_NEAREST;

    GLenum target = static_cast<GLenum>(m_type);

    if (m_type == TextureType::Cubemap) {
        for (GLuint face = 0; face < 6; ++face) {
            GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, faceTarget, src->id(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, faceTarget, m_id, 0);

            if (attachment == GL_COLOR_ATTACHMENT0) {
                glReadBuffer(attachment);
                glDrawBuffer(attachment);
            }

            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, mask, filter);
        }
    }
    else {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, target, src->id(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, target, m_id, 0);

        if (attachment == GL_COLOR_ATTACHMENT0) {
            glReadBuffer(attachment);
            glDrawBuffer(attachment);
        }

        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, mask, filter);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &fboRead);
    glDeleteFramebuffers(1, &fboDraw);
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
