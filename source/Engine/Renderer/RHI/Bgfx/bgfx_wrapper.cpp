#include "bgfx_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <iostream>
#include <cstring>

bool BgfxWrapper::s_initialized = false;
SDL_Window* BgfxWrapper::s_window = nullptr;
bgfx::PlatformData BgfxWrapper::s_platformData = {};

bool BgfxWrapper::Initialize(SDL_Window* window)
{
    if (s_initialized) {
        return true;
    }
    
    s_window = window;
    
    // Initialize bgfx platform data
    s_platformData.ndt = nullptr;
    s_platformData.nwh = SDL_GetWindowWMInfo(window, &s_platformData.wmi) ? s_platformData.wmi.info.win.window : nullptr;
    s_platformData.context = nullptr;
    s_platformData.backBuffer = nullptr;
    s_platformData.backBufferDS = nullptr;
    
    // Set platform data
    bgfx::setPlatformData(s_platformData);
    
    // Initialize bgfx
    if (!bgfx::init()) {
        std::cerr << "Failed to initialize bgfx!" << std::endl;
        return false;
    }
    
    // Get window size
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    
    // Reset with default settings
    bgfx::reset(width, height, BGFX_RESET_VSYNC);
    
    // Enable debug text
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    
    s_initialized = true;
    return true;
}

void BgfxWrapper::Shutdown()
{
    if (s_initialized) {
        bgfx::shutdown();
        s_initialized = false;
        s_window = nullptr;
    }
}

void BgfxWrapper::BeginFrame()
{
    if (s_initialized) {
        bgfx::frame();
    }
}

void BgfxWrapper::EndFrame()
{
    // bgfx handles frame ending internally
}

void BgfxWrapper::SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    bgfx::setViewRect(0, x, y, width, height);
}

void BgfxWrapper::SetViewClear(uint8_t viewId, uint16_t flags, uint32_t rgba, float depth, uint8_t stencil)
{
    bgfx::setViewClear(viewId, flags, rgba, depth, stencil);
}

void BgfxWrapper::Touch(uint8_t viewId)
{
    bgfx::touch(viewId);
}

void BgfxWrapper::SetState(uint64_t state, uint32_t rgba)
{
    bgfx::setState(state, rgba);
}

void BgfxWrapper::SetTransform(const void* mtx)
{
    bgfx::setTransform(mtx);
}

void BgfxWrapper::SetVertexBuffer(uint8_t stream, bgfx::VertexBufferHandle handle, uint32_t startVertex, uint32_t numVertices)
{
    bgfx::setVertexBuffer(stream, handle, startVertex, numVertices);
}

void BgfxWrapper::SetIndexBuffer(bgfx::IndexBufferHandle handle, uint32_t firstIndex, uint32_t numIndices)
{
    bgfx::setIndexBuffer(handle, firstIndex, numIndices);
}

void BgfxWrapper::SetInstanceDataBuffer(const bgfx::InstanceDataBuffer* idb, uint32_t start, uint32_t num)
{
    bgfx::setInstanceDataBuffer(idb, start, num);
}

bgfx::ShaderHandle BgfxWrapper::CreateShader(const void* data, uint32_t size)
{
    return bgfx::createShader(bgfx::copy(data, size));
}

bgfx::ShaderHandle BgfxWrapper::CreateShaderFromGLSL(const char* source, uint32_t shaderType)
{
    const bgfx::Memory* mem = CompileGLSL(source, shaderType);
    if (mem) {
        return bgfx::createShader(mem);
    }
    return BGFX_INVALID_HANDLE;
}

const bgfx::Memory* BgfxWrapper::CompileGLSL(const char* source, uint32_t shaderType)
{
    // For now, we'll use a simple approach where we assume the shader is already compiled
    // In a real implementation, you would use bgfx's shader compiler or a third-party GLSL compiler
    // This is a placeholder that copies the source as-is
    uint32_t size = (uint32_t)strlen(source);
    const bgfx::Memory* mem = bgfx::alloc(size + 1);
    memcpy(mem->data, source, size);
    mem->data[size] = '\0';
    return mem;
}

uint32_t BgfxWrapper::GetShaderType(uint32_t glShaderType)
{
    switch (glShaderType) {
        case 0x8B31: // GL_VERTEX_SHADER
            return 0; // Vertex shader
        case 0x8B30: // GL_FRAGMENT_SHADER
            return 1; // Fragment shader
        case 0x8DD9: // GL_GEOMETRY_SHADER
            return 2; // Geometry shader
        case 0x8E88: // GL_TESS_CONTROL_SHADER
            return 3; // Tessellation control shader
        case 0x8E89: // GL_TESS_EVALUATION_SHADER
            return 4; // Tessellation evaluation shader
        case 0x91B9: // GL_COMPUTE_SHADER
            return 5; // Compute shader
        default:
            return 0; // Default to vertex shader
    }
}

void BgfxWrapper::DestroyShader(bgfx::ShaderHandle handle)
{
    bgfx::destroy(handle);
}

bgfx::ProgramHandle BgfxWrapper::CreateProgram(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh, bool destroyShaders)
{
    return bgfx::createProgram(vsh, fsh, destroyShaders);
}

void BgfxWrapper::DestroyProgram(bgfx::ProgramHandle handle)
{
    bgfx::destroy(handle);
}

void BgfxWrapper::Submit(uint8_t viewId, bgfx::ProgramHandle program, uint32_t depth, uint8_t flags)
{
    bgfx::submit(viewId, program, depth, flags);
}

bgfx::TextureHandle BgfxWrapper::CreateTexture2D(uint16_t width, uint16_t height, bool hasMips, uint16_t numLayers, bgfx::TextureFormat::Enum format, uint64_t flags, const bgfx::Memory* mem)
{
    return bgfx::createTexture2D(width, height, hasMips, numLayers, format, flags, mem);
}

bgfx::TextureHandle BgfxWrapper::CreateTextureCube(uint16_t size, bool hasMips, uint16_t numLayers, bgfx::TextureFormat::Enum format, uint64_t flags, const bgfx::Memory* mem)
{
    return bgfx::createTextureCube(size, hasMips, numLayers, format, flags, mem);
}

void BgfxWrapper::DestroyTexture(bgfx::TextureHandle handle)
{
    bgfx::destroy(handle);
}

void BgfxWrapper::SetTexture(uint8_t stage, bgfx::UniformHandle sampler, bgfx::TextureHandle handle, uint32_t flags)
{
    bgfx::setTexture(stage, sampler, handle, flags);
}

void BgfxWrapper::SetTextureFromAttachment(uint8_t stage, uint32_t handle, uint32_t flags)
{
    // For now, this is a placeholder - bgfx handles attachments differently
    // In a real implementation, you would use bgfx::setTexture with the appropriate handle
}

bgfx::UniformHandle BgfxWrapper::CreateUniform(const char* name, bgfx::UniformType::Enum type, uint16_t num)
{
    return bgfx::createUniform(name, type, num);
}

void BgfxWrapper::DestroyUniform(bgfx::UniformHandle handle)
{
    bgfx::destroy(handle);
}

void BgfxWrapper::SetUniform(bgfx::UniformHandle handle, const void* value, uint16_t num)
{
    bgfx::setUniform(handle, value, num);
}

bgfx::VertexBufferHandle BgfxWrapper::CreateVertexBuffer(const bgfx::Memory* mem, const bgfx::VertexLayout& decl, uint16_t flags)
{
    return bgfx::createVertexBuffer(mem, decl, flags);
}

void BgfxWrapper::DestroyVertexBuffer(bgfx::VertexBufferHandle handle)
{
    bgfx::destroy(handle);
}

bgfx::IndexBufferHandle BgfxWrapper::CreateIndexBuffer(const bgfx::Memory* mem, uint16_t flags)
{
    return bgfx::createIndexBuffer(mem, flags);
}

void BgfxWrapper::DestroyIndexBuffer(bgfx::IndexBufferHandle handle)
{
    bgfx::destroy(handle);
}

bgfx::FrameBufferHandle BgfxWrapper::CreateFrameBuffer(uint16_t width, uint16_t height, bgfx::TextureFormat::Enum format, uint64_t textureFlags)
{
    return bgfx::createFrameBuffer(width, height, format, textureFlags);
}

bgfx::FrameBufferHandle BgfxWrapper::CreateFrameBuffer(uint8_t num, const bgfx::Attachment* attachment, bool destroyTextures)
{
    return bgfx::createFrameBuffer(num, attachment, destroyTextures);
}

void BgfxWrapper::DestroyFrameBuffer(bgfx::FrameBufferHandle handle)
{
    bgfx::destroy(handle);
}

void BgfxWrapper::SetFrameBuffer(uint8_t viewId, bgfx::FrameBufferHandle handle)
{
    bgfx::setViewFrameBuffer(viewId, handle);
}

const bgfx::Memory* BgfxWrapper::Alloc(uint32_t size)
{
    return bgfx::alloc(size);
}

void BgfxWrapper::Free(const bgfx::Memory* mem)
{
    bgfx::free(mem);
}

void BgfxWrapper::SetDebug(uint32_t debug)
{
    bgfx::setDebug(debug);
}

void BgfxWrapper::SetViewTransform(uint8_t viewId, const void* view, const void* proj)
{
    bgfx::setViewTransform(viewId, view, proj);
}

void BgfxWrapper::SetViewRect(uint8_t viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    bgfx::setViewRect(viewId, x, y, width, height);
}

void BgfxWrapper::SetPlatformData(const bgfx::PlatformData& data)
{
    s_platformData = data;
    bgfx::setPlatformData(s_platformData);
}

void BgfxWrapper::Reset(uint32_t width, uint32_t height, uint32_t flags, bgfx::TextureFormat::Enum depthFormat)
{
    bgfx::reset(width, height, flags, depthFormat);
}
