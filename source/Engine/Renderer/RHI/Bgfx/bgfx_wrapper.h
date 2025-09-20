#pragma once

#include <bgfx/bgfx.h>

#include <cstdint>
#include <cstring>
#include <SDL2/SDL.h>

// bgfx wrapper class to provide a clean interface
class BgfxWrapper
{
public:
    // Initialization and cleanup
    static bool Initialize(SDL_Window* window);
    static void Shutdown();
    static void BeginFrame();
    static void EndFrame();
    
    // Viewport and clear
    static void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    static void SetViewClear(uint8_t viewId, uint16_t flags, uint32_t rgba, float depth, uint8_t stencil);
    static void Touch(uint8_t viewId);
    
    // State management
    static void SetState(uint64_t state, uint32_t rgba = 0);
    static void SetTransform(const void* mtx);
    static void SetVertexBuffer(uint8_t stream, bgfx::VertexBufferHandle handle, uint32_t startVertex, uint32_t numVertices);
    static void SetIndexBuffer(bgfx::IndexBufferHandle handle, uint32_t firstIndex, uint32_t numIndices);
    static void SetInstanceDataBuffer(const bgfx::InstanceDataBuffer* idb, uint32_t start, uint32_t num);
    
    // Shader and program management
    static bgfx::ShaderHandle CreateShader(const void* data, uint32_t size);
    static bgfx::ShaderHandle CreateShaderFromGLSL(const char* source, uint32_t shaderType);
    static void DestroyShader(bgfx::ShaderHandle handle);
    static bgfx::ProgramHandle CreateProgram(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh, bool destroyShaders = false);
    static void DestroyProgram(bgfx::ProgramHandle handle);
    static void Submit(uint8_t viewId, bgfx::ProgramHandle program, uint32_t depth = 0, uint8_t flags = BGFX_DISCARD_ALL);
    
    // GLSL shader compilation helpers
    static const bgfx::Memory* CompileGLSL(const char* source, uint32_t shaderType);
    static uint32_t GetShaderType(uint32_t glShaderType);
    
    // Texture management
    static bgfx::TextureHandle CreateTexture2D(uint16_t width, uint16_t height, bool hasMips, uint16_t numLayers, bgfx::TextureFormat::Enum format, uint64_t flags, const bgfx::Memory* mem);
    static bgfx::TextureHandle CreateTextureCube(uint16_t size, bool hasMips, uint16_t numLayers, bgfx::TextureFormat::Enum format, uint64_t flags, const bgfx::Memory* mem);
    static void DestroyTexture(bgfx::TextureHandle handle);
    static void SetTexture(uint8_t stage, bgfx::UniformHandle sampler, bgfx::TextureHandle handle, uint32_t flags = UINT32_MAX);
    static void SetTextureFromAttachment(uint8_t stage, uint32_t handle, uint32_t flags = UINT32_MAX);
    
    // Uniform management
    static bgfx::UniformHandle CreateUniform(const char* name, bgfx::UniformType::Enum type, uint16_t num = 1);
    static void DestroyUniform(bgfx::UniformHandle handle);
    static void SetUniform(bgfx::UniformHandle handle, const void* value, uint16_t num = 1);
    
    // Buffer management
    static bgfx::VertexBufferHandle CreateVertexBuffer(const bgfx::Memory* mem, const bgfx::VertexLayout& decl, uint16_t flags = BGFX_BUFFER_NONE);
    static void DestroyVertexBuffer(bgfx::VertexBufferHandle handle);
    static bgfx::IndexBufferHandle CreateIndexBuffer(const bgfx::Memory* mem, uint16_t flags = BGFX_BUFFER_NONE);
    static void DestroyIndexBuffer(bgfx::IndexBufferHandle handle);
    
    // Frame buffer management
    static bgfx::FrameBufferHandle CreateFrameBuffer(uint16_t width, uint16_t height, bgfx::TextureFormat::Enum format, uint64_t textureFlags = BGFX_TEXTURE_RT);
    static bgfx::FrameBufferHandle CreateFrameBuffer(uint8_t num, const bgfx::Attachment* attachment, bool destroyTextures = false);
    static void DestroyFrameBuffer(bgfx::FrameBufferHandle handle);
    static void SetFrameBuffer(uint8_t viewId, bgfx::FrameBufferHandle handle);
    
    // Utility functions
    static const bgfx::Memory* Alloc(uint32_t size);
    static void Free(const bgfx::Memory* mem);
    static void SetDebug(uint32_t debug);
    static void SetViewTransform(uint8_t viewId, const void* view, const void* proj);
    static void SetViewRect(uint8_t viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    
    // Platform-specific functions
    static void SetPlatformData(const bgfx::PlatformData& data);
    static void Reset(uint32_t width, uint32_t height, uint32_t flags = BGFX_RESET_VSYNC, bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::D24S8);
    
private:
    static bool s_initialized;
    static SDL_Window* s_window;
    static bgfx::PlatformData s_platformData;
};

// Convenience macros
#define BGFX_WRAPPER BgfxWrapper
