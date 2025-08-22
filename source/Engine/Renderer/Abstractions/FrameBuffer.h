#pragma once
#include "RenderTexture.h"
#include <vector>
#include <stdexcept>

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // Attachment management
    void attachColor(RenderTexture* texture, uint32_t attachmentIndex = 0);
    void attachDepth(RenderTexture* texture);
    void attachCubemapFace(RenderTexture* cubemap, uint32_t face, bool isDepth = false);

    void resolve(Framebuffer& target, GLbitfield mask = GL_COLOR_BUFFER_BIT,
        GLenum filter = GL_LINEAR);

    // State management
    void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
    static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    // Attachments
    RenderTexture* colorAttachment(uint32_t index = 0) const;
    RenderTexture* depthAttachment() const { return m_depthAttachment; }

    GLuint id() { return m_id; }

private:
    GLuint m_id;
    std::vector<RenderTexture*> m_colorAttachments;
    RenderTexture* m_depthAttachment = nullptr;

    void validate() const;
    void updateDrawBuffers() const;
};