// Framebuffer.cpp
#include "FrameBuffer.h"
#include "RenderTexture.h"
#include "../../gl.h"
#include <stdexcept>
#include <algorithm>
#include <string>

//------------------------------------------------------------------------------
// On WebGL only COLOR_ATTACHMENT0 is allowed, no MRT
#if defined(__EMSCRIPTEN__)
#define DISABLE_MRT
#endif

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &m_id);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_id);
}

void Framebuffer::attachColor(RenderTexture* texture,
    uint32_t attachmentIndex)
{
    if (!texture) throw std::invalid_argument("Color attachment is null");

    if (attachmentIndex >= m_colorAttachments.size())
        m_colorAttachments.resize(attachmentIndex + 1, nullptr);
    m_colorAttachments[attachmentIndex] = texture;

    bind();
    GLenum target = static_cast<GLenum>(texture->type());
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + attachmentIndex,
        target,
        texture->id(),
        0
    );

#if !defined(DISABLE_MRT)
    updateDrawBuffers();
#endif

    validate();
    unbind();
}

void Framebuffer::attachDepth(RenderTexture* texture) {
    if (!texture) throw std::invalid_argument("Depth attachment is null");
    m_depthAttachment = texture;

    bind();
    GLenum target = static_cast<GLenum>(texture->type());
    GLenum attach = (texture->format() == TextureFormat::Depth24Stencil8 ||
        texture->format() == TextureFormat::Depth32FStencil8)
        ? GL_DEPTH_STENCIL_ATTACHMENT
        : GL_DEPTH_ATTACHMENT;

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        attach,
        target,
        texture->id(),
        0
    );

    validate();
    unbind();
}

void Framebuffer::attachCubemapFace(RenderTexture* cubemap,
    uint32_t face,
    bool isDepth)
{
    if (!cubemap) throw std::invalid_argument("Cubemap is null");
    if (cubemap->type() != TextureType::Cubemap)
        throw std::invalid_argument("Not a cubemap texture");
    if (face > 5) throw std::out_of_range("Cubemap face index 0–5");

    bind();
    GLenum targetFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
    GLenum attachPt = isDepth
        ? GL_DEPTH_ATTACHMENT
        : GL_COLOR_ATTACHMENT0;

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        attachPt,
        targetFace,
        cubemap->id(),
        0
    );

    if (isDepth) {
        m_depthAttachment = cubemap;
    }
    else {
        if (m_colorAttachments.empty())
            m_colorAttachments.resize(1, nullptr);
        m_colorAttachments[0] = cubemap;
    }

#if !defined(DISABLE_MRT)
    if (!isDepth)
        updateDrawBuffers();
#endif

    validate();
    unbind();
}

void Framebuffer::resolve(Framebuffer& target,
    GLbitfield mask,
    GLenum filter)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.m_id);

    GLsizei srcW = m_colorAttachments[0]->width();
    GLsizei srcH = m_colorAttachments[0]->height();
    GLsizei dstW = target.m_colorAttachments[0]->width();
    GLsizei dstH = target.m_colorAttachments[0]->height();

    // Color
    if (mask & GL_COLOR_BUFFER_BIT) {
        glBlitFramebuffer(
            0, 0, srcW, srcH,
            0, 0, dstW, dstH,
            GL_COLOR_BUFFER_BIT,
            filter
        );
    }
    // Depth
    if (mask & GL_DEPTH_BUFFER_BIT) {
        glBlitFramebuffer(
            0, 0, srcW, srcH,
            0, 0, dstW, dstH,
            GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
    }
    // Stencil
    if (mask & GL_STENCIL_BUFFER_BIT) {
        glBlitFramebuffer(
            0, 0, srcW, srcH,
            0, 0, dstW, dstH,
            GL_STENCIL_BUFFER_BIT,
            GL_NEAREST
        );
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

RenderTexture* Framebuffer::colorAttachment(uint32_t index) const {
    if (index >= m_colorAttachments.size() || !m_colorAttachments[index])
        throw std::out_of_range("No color attachment at index");
    return m_colorAttachments[index];
}

void Framebuffer::validate() const {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(
            "Framebuffer incomplete: status code " +
            std::to_string(status)
        );
    }
}

void Framebuffer::updateDrawBuffers() const {
#if defined(DISABLE_MRT)
    // WebGL only has a single COLOR_ATTACHMENT0 by default — skip
#else
    std::vector<GLenum> buffers;
    buffers.reserve(m_colorAttachments.size());
    for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
        buffers.push_back(
            m_colorAttachments[i]
            ? GL_COLOR_ATTACHMENT0 + (GLenum)i
            : GL_NONE
        );
    }
    glDrawBuffers(
        static_cast<GLsizei>(buffers.size()),
        buffers.data()
    );
#endif
}
