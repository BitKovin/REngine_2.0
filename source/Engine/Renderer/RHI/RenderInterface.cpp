#include "RenderInterface.h"
#include "Bgfx/bgfx_wrapper.h"
#include <iostream>
#include <cstring>

// Static member definitions
RenderInterface::Backend RenderInterface::s_currentBackend = RenderInterface::Backend::Bgfx;
bool RenderInterface::s_initialized = false;
SDL_Window* RenderInterface::s_window = nullptr;

bool RenderInterface::Initialize(SDL_Window* window, Backend backend)
{
    if (s_initialized) {
        return true;
    }
    
    s_window = window;
    s_currentBackend = backend;
    
    switch (backend) {
        case Backend::Bgfx:
            return BgfxWrapper::Initialize(window);
        case Backend::OpenGL:
            // OpenGL initialization would go here
            // For now, we'll use bgfx as the default
            return BgfxWrapper::Initialize(window);
        default:
            std::cerr << "Unknown render backend!" << std::endl;
            return false;
    }
}

void RenderInterface::Shutdown()
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::Shutdown();
                break;
            case Backend::OpenGL:
                // OpenGL cleanup would go here
                break;
        }
        s_initialized = false;
        s_window = nullptr;
    }
}

void RenderInterface::BeginFrame()
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::BeginFrame();
                break;
            case Backend::OpenGL:
                // OpenGL frame begin would go here
                break;
        }
    }
}

void RenderInterface::EndFrame()
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::EndFrame();
                break;
            case Backend::OpenGL:
                // OpenGL frame end would go here
                break;
        }
    }
}

void RenderInterface::SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetViewport(x, y, width, height);
                break;
            case Backend::OpenGL:
                // OpenGL viewport would go here
                break;
        }
    }
}

void RenderInterface::SetViewClear(uint8_t viewId, uint16_t flags, uint32_t rgba, float depth, uint8_t stencil)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetViewClear(viewId, flags, rgba, depth, stencil);
                break;
            case Backend::OpenGL:
                // OpenGL clear would go here
                break;
        }
    }
}

void RenderInterface::Touch(uint8_t viewId)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::Touch(viewId);
                break;
            case Backend::OpenGL:
                // OpenGL touch would go here
                break;
        }
    }
}

void RenderInterface::SetState(uint64_t state, uint32_t rgba)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetState(state, rgba);
                break;
            case Backend::OpenGL:
                // OpenGL state would go here
                break;
        }
    }
}

void RenderInterface::SetTransform(const void* mtx)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetTransform(mtx);
                break;
            case Backend::OpenGL:
                // OpenGL transform would go here
                break;
        }
    }
}

void RenderInterface::SetVertexBuffer(uint8_t stream, uint32_t handle, uint32_t startVertex, uint32_t numVertices)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetVertexBuffer(stream, (bgfx::VertexBufferHandle)handle, startVertex, numVertices);
                break;
            case Backend::OpenGL:
                // OpenGL vertex buffer would go here
                break;
        }
    }
}

void RenderInterface::SetIndexBuffer(uint32_t handle, uint32_t firstIndex, uint32_t numIndices)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetIndexBuffer((bgfx::IndexBufferHandle)handle, firstIndex, numIndices);
                break;
            case Backend::OpenGL:
                // OpenGL index buffer would go here
                break;
        }
    }
}

void RenderInterface::SetInstanceDataBuffer(const void* idb, uint32_t start, uint32_t num)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetInstanceDataBuffer((const bgfx::InstanceDataBuffer*)idb, start, num);
                break;
            case Backend::OpenGL:
                // OpenGL instance data would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateShader(const void* data, uint32_t size, uint32_t shaderType)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateShader(data, size);
            case Backend::OpenGL:
                // OpenGL shader creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

uint32_t RenderInterface::CreateShaderFromGLSL(const char* source, uint32_t shaderType)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateShaderFromGLSL(source, shaderType);
            case Backend::OpenGL:
                // OpenGL shader creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyShader(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyShader((bgfx::ShaderHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL shader destruction would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateProgram(uint32_t vsh, uint32_t fsh, bool destroyShaders)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateProgram((bgfx::ShaderHandle)vsh, (bgfx::ShaderHandle)fsh, destroyShaders);
            case Backend::OpenGL:
                // OpenGL program creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyProgram(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyProgram((bgfx::ProgramHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL program destruction would go here
                break;
        }
    }
}

void RenderInterface::Submit(uint8_t viewId, uint32_t program, uint32_t depth, uint8_t flags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::Submit(viewId, (bgfx::ProgramHandle)program, depth, flags);
                break;
            case Backend::OpenGL:
                // OpenGL submit would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateTexture2D(uint16_t width, uint16_t height, bool hasMips, uint16_t numLayers, uint32_t format, uint64_t flags, const void* mem)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateTexture2D(width, height, hasMips, numLayers, (bgfx::TextureFormat::Enum)format, flags, (const bgfx::Memory*)mem);
            case Backend::OpenGL:
                // OpenGL texture creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

uint32_t RenderInterface::CreateTextureCube(uint16_t size, bool hasMips, uint16_t numLayers, uint32_t format, uint64_t flags, const void* mem)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateTextureCube(size, hasMips, numLayers, (bgfx::TextureFormat::Enum)format, flags, (const bgfx::Memory*)mem);
            case Backend::OpenGL:
                // OpenGL texture cube creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyTexture(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyTexture((bgfx::TextureHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL texture destruction would go here
                break;
        }
    }
}

void RenderInterface::SetTexture(uint8_t stage, uint32_t sampler, uint32_t handle, uint32_t flags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetTexture(stage, (bgfx::UniformHandle)sampler, (bgfx::TextureHandle)handle, flags);
                break;
            case Backend::OpenGL:
                // OpenGL texture binding would go here
                break;
        }
    }
}

void RenderInterface::SetTextureFromAttachment(uint8_t stage, uint32_t handle, uint32_t flags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetTextureFromAttachment(stage, handle, flags);
                break;
            case Backend::OpenGL:
                // OpenGL texture from attachment would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateUniform(const char* name, uint32_t type, uint16_t num)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateUniform(name, (bgfx::UniformType::Enum)type, num);
            case Backend::OpenGL:
                // OpenGL uniform creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyUniform(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyUniform((bgfx::UniformHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL uniform destruction would go here
                break;
        }
    }
}

void RenderInterface::SetUniform(uint32_t handle, const void* value, uint16_t num)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetUniform((bgfx::UniformHandle)handle, value, num);
                break;
            case Backend::OpenGL:
                // OpenGL uniform setting would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateVertexBuffer(const void* mem, uint32_t size, const void* decl, uint16_t flags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateVertexBuffer((const bgfx::Memory*)mem, *(const bgfx::VertexLayout*)decl, flags);
            case Backend::OpenGL:
                // OpenGL vertex buffer creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyVertexBuffer(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyVertexBuffer((bgfx::VertexBufferHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL vertex buffer destruction would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateIndexBuffer(const void* mem, uint32_t size, uint16_t flags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateIndexBuffer((const bgfx::Memory*)mem, flags);
            case Backend::OpenGL:
                // OpenGL index buffer creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyIndexBuffer(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyIndexBuffer((bgfx::IndexBufferHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL index buffer destruction would go here
                break;
        }
    }
}

uint32_t RenderInterface::CreateFrameBuffer(uint16_t width, uint16_t height, uint32_t format, uint64_t textureFlags)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateFrameBuffer(width, height, (bgfx::TextureFormat::Enum)format, textureFlags);
            case Backend::OpenGL:
                // OpenGL framebuffer creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

uint32_t RenderInterface::CreateFrameBuffer(uint8_t num, const void* attachment, bool destroyTextures)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (uint32_t)BgfxWrapper::CreateFrameBuffer(num, (const bgfx::Attachment*)attachment, destroyTextures);
            case Backend::OpenGL:
                // OpenGL framebuffer creation would go here
                return INVALID_HANDLE;
        }
    }
    return INVALID_HANDLE;
}

void RenderInterface::DestroyFrameBuffer(uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::DestroyFrameBuffer((bgfx::FrameBufferHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL framebuffer destruction would go here
                break;
        }
    }
}

void RenderInterface::SetFrameBuffer(uint8_t viewId, uint32_t handle)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetFrameBuffer(viewId, (bgfx::FrameBufferHandle)handle);
                break;
            case Backend::OpenGL:
                // OpenGL framebuffer binding would go here
                break;
        }
    }
}

void* RenderInterface::Alloc(uint32_t size)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                return (void*)BgfxWrapper::Alloc(size);
            case Backend::OpenGL:
                // OpenGL memory allocation would go here
                return nullptr;
        }
    }
    return nullptr;
}

void RenderInterface::Free(void* mem)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::Free((const bgfx::Memory*)mem);
                break;
            case Backend::OpenGL:
                // OpenGL memory freeing would go here
                break;
        }
    }
}

void RenderInterface::SetDebug(uint32_t debug)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetDebug(debug);
                break;
            case Backend::OpenGL:
                // OpenGL debug would go here
                break;
        }
    }
}

void RenderInterface::SetViewTransform(uint8_t viewId, const void* view, const void* proj)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetViewTransform(viewId, view, proj);
                break;
            case Backend::OpenGL:
                // OpenGL view transform would go here
                break;
        }
    }
}

void RenderInterface::SetViewRect(uint8_t viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::SetViewRect(viewId, x, y, width, height);
                break;
            case Backend::OpenGL:
                // OpenGL view rect would go here
                break;
        }
    }
}

void RenderInterface::Reset(uint32_t width, uint32_t height, uint32_t flags, uint32_t depthFormat)
{
    if (s_initialized) {
        switch (s_currentBackend) {
            case Backend::Bgfx:
                BgfxWrapper::Reset(width, height, flags, (bgfx::TextureFormat::Enum)depthFormat);
                break;
            case Backend::OpenGL:
                // OpenGL reset would go here
                break;
        }
    }
}

// Drawing functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::DrawArrays(uint32_t primitive, uint32_t first, uint32_t count)
{
    // This is handled by bgfx's submit system
}

void RenderInterface::DrawElements(uint32_t primitive, uint32_t count, uint32_t type, const void* indices)
{
    // This is handled by bgfx's submit system
}

void RenderInterface::DrawElementsInstanced(uint32_t primitive, uint32_t count, uint32_t type, const void* indices, uint32_t instanceCount)
{
    // This is handled by bgfx's submit system
}

// State functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::Enable(uint32_t capability)
{
    // This is handled by bgfx's state system
}

void RenderInterface::Disable(uint32_t capability)
{
    // This is handled by bgfx's state system
}

void RenderInterface::BlendFunc(uint32_t sfactor, uint32_t dfactor)
{
    // This is handled by bgfx's state system
}

void RenderInterface::DepthFunc(uint32_t func)
{
    // This is handled by bgfx's state system
}

void RenderInterface::DepthMask(bool flag)
{
    // This is handled by bgfx's state system
}

void RenderInterface::ColorMask(bool r, bool g, bool b, bool a)
{
    // This is handled by bgfx's state system
}

void RenderInterface::Clear(uint32_t mask)
{
    // This is handled by bgfx's clear system
}

void RenderInterface::ClearColor(float r, float g, float b, float a)
{
    // This is handled by bgfx's clear system
}

void RenderInterface::PolygonOffset(float factor, float units)
{
    // This is handled by bgfx's state system
}

void RenderInterface::PolygonMode(uint32_t face, uint32_t mode)
{
    // This is handled by bgfx's state system
}

// Vertex array functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::GenVertexArrays(uint32_t n, uint32_t* arrays)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::DeleteVertexArrays(uint32_t n, const uint32_t* arrays)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::BindVertexArray(uint32_t array)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::EnableVertexAttribArray(uint32_t index)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::DisableVertexAttribArray(uint32_t index)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::VertexAttribPointer(uint32_t index, int32_t size, uint32_t type, bool normalized, int32_t stride, const void* pointer)
{
    // This is handled by bgfx's vertex layout system
}

void RenderInterface::VertexAttribDivisor(uint32_t index, uint32_t divisor)
{
    // This is handled by bgfx's vertex layout system
}

// Buffer functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::GenBuffers(uint32_t n, uint32_t* buffers)
{
    // This is handled by bgfx's buffer system
}

void RenderInterface::DeleteBuffers(uint32_t n, const uint32_t* buffers)
{
    // This is handled by bgfx's buffer system
}

void RenderInterface::BindBuffer(uint32_t target, uint32_t buffer)
{
    // This is handled by bgfx's buffer system
}

void RenderInterface::BufferData(uint32_t target, int32_t size, const void* data, uint32_t usage)
{
    // This is handled by bgfx's buffer system
}

void RenderInterface::BufferSubData(uint32_t target, int32_t offset, int32_t size, const void* data)
{
    // This is handled by bgfx's buffer system
}

// Texture functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::GenTextures(uint32_t n, uint32_t* textures)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::DeleteTextures(uint32_t n, const uint32_t* textures)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::BindTexture(uint32_t target, uint32_t texture)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::ActiveTexture(uint32_t texture)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::TexImage2D(uint32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void* pixels)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::TexParameteri(uint32_t target, uint32_t pname, int32_t param)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::TexParameterf(uint32_t target, uint32_t pname, float param)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::GenerateMipmap(uint32_t target)
{
    // This is handled by bgfx's texture system
}

void RenderInterface::PixelStorei(uint32_t pname, int32_t param)
{
    // This is handled by bgfx's texture system
}

// Program functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::UseProgram(uint32_t program)
{
    // This is handled by bgfx's program system
}

int32_t RenderInterface::GetUniformLocation(uint32_t program, const char* name)
{
    // This is handled by bgfx's uniform system
    return -1;
}

void RenderInterface::Uniform1i(int32_t location, int32_t v0)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform1f(int32_t location, float v0)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform2f(int32_t location, float v0, float v1)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform3f(int32_t location, float v0, float v1, float v2)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform4f(int32_t location, float v0, float v1, float v2, float v3)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::UniformMatrix2fv(int32_t location, int32_t count, bool transpose, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::UniformMatrix3fv(int32_t location, int32_t count, bool transpose, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::UniformMatrix4fv(int32_t location, int32_t count, bool transpose, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform1fv(int32_t location, int32_t count, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform1iv(int32_t location, int32_t count, const int32_t* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform2fv(int32_t location, int32_t count, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform3fv(int32_t location, int32_t count, const float* value)
{
    // This is handled by bgfx's uniform system
}

void RenderInterface::Uniform4fv(int32_t location, int32_t count, const float* value)
{
    // This is handled by bgfx's uniform system
}

// Frame buffer functions - these are OpenGL-specific and will be implemented as no-ops for bgfx
void RenderInterface::GenFramebuffers(uint32_t n, uint32_t* framebuffers)
{
    // This is handled by bgfx's framebuffer system
}

void RenderInterface::DeleteFramebuffers(uint32_t n, const uint32_t* framebuffers)
{
    // This is handled by bgfx's framebuffer system
}

void RenderInterface::BindFramebuffer(uint32_t target, uint32_t framebuffer)
{
    // This is handled by bgfx's framebuffer system
}

void RenderInterface::FramebufferTexture2D(uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, int32_t level)
{
    // This is handled by bgfx's framebuffer system
}

void RenderInterface::BlitFramebuffer(int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, uint32_t mask, uint32_t filter)
{
    // This is handled by bgfx's framebuffer system
}

// Error checking - these are OpenGL-specific and will be implemented as no-ops for bgfx
uint32_t RenderInterface::GetError()
{
    // This is handled by bgfx's error system
    return NO_ERROR;
}

void RenderInterface::GetIntegerv(uint32_t pname, int32_t* data)
{
    // This is handled by bgfx's state system
}

void RenderInterface::GetFloatv(uint32_t pname, float* data)
{
    // This is handled by bgfx's state system
}
