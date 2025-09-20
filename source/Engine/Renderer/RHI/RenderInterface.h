#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace bgfx
{
    // Forward declarations
    struct VertexLayout;
    struct Memory;
    struct Attachment;
}


// Universal render interface that abstracts both OpenGL and bgfx
class RenderInterface
{
public:
    enum class Backend
    {
        OpenGL,
        Bgfx
    };

    // Initialization and cleanup
    static bool Initialize(SDL_Window* window, Backend backend = Backend::Bgfx);
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
    static void SetVertexBuffer(uint8_t stream, uint32_t handle, uint32_t startVertex, uint32_t numVertices);
    static void SetIndexBuffer(uint32_t handle, uint32_t firstIndex, uint32_t numIndices);
    static void SetInstanceDataBuffer(const void* idb, uint32_t start, uint32_t num);
    
    // Shader and program management
    static uint32_t CreateShader(const void* data, uint32_t size, uint32_t shaderType);
    static uint32_t CreateShaderFromGLSL(const char* source, uint32_t shaderType);
    static void DestroyShader(uint32_t handle);
    static uint32_t CreateProgram(uint32_t vsh, uint32_t fsh, bool destroyShaders = false);
    static void DestroyProgram(uint32_t handle);
    static void Submit(uint8_t viewId, uint32_t program, uint32_t depth = 0, uint8_t flags = 0);
    
    // Texture management
    static uint32_t CreateTexture2D(uint16_t width, uint16_t height, bool hasMips, uint16_t numLayers, uint32_t format, uint64_t flags, const void* mem);
    static uint32_t CreateTextureCube(uint16_t size, bool hasMips, uint16_t numLayers, uint32_t format, uint64_t flags, const void* mem);
    static void DestroyTexture(uint32_t handle);
    static void SetTexture(uint8_t stage, uint32_t sampler, uint32_t handle, uint32_t flags = UINT32_MAX);
    static void SetTextureFromAttachment(uint8_t stage, uint32_t handle, uint32_t flags = UINT32_MAX);
    
    // Uniform management
    static uint32_t CreateUniform(const char* name, uint32_t type, uint16_t num = 1);
    static void DestroyUniform(uint32_t handle);
    static void SetUniform(uint32_t handle, const void* value, uint16_t num = 1);
    
    // Buffer management
    static uint32_t CreateVertexBuffer(const void* mem, uint32_t size, const void* decl, uint16_t flags = 0);
    static void DestroyVertexBuffer(uint32_t handle);
    static uint32_t CreateIndexBuffer(const void* mem, uint32_t size, uint16_t flags = 0);
    static void DestroyIndexBuffer(uint32_t handle);
    
    // Frame buffer management
    static uint32_t CreateFrameBuffer(uint16_t width, uint16_t height, uint32_t format, uint64_t textureFlags = 0);
    static uint32_t CreateFrameBuffer(uint8_t num, const void* attachment, bool destroyTextures = false);
    static void DestroyFrameBuffer(uint32_t handle);
    static void SetFrameBuffer(uint8_t viewId, uint32_t handle);
    
    // Utility functions
    static void* Alloc(uint32_t size);
    static void Free(void* mem);
    static void SetDebug(uint32_t debug);
    static void SetViewTransform(uint8_t viewId, const void* view, const void* proj);
    static void SetViewRect(uint8_t viewId, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    static void Reset(uint32_t width, uint32_t height, uint32_t flags = 0, uint32_t depthFormat = 0);
    
    // Drawing functions
    static void DrawArrays(uint32_t primitive, uint32_t first, uint32_t count);
    static void DrawElements(uint32_t primitive, uint32_t count, uint32_t type, const void* indices);
    static void DrawElementsInstanced(uint32_t primitive, uint32_t count, uint32_t type, const void* indices, uint32_t instanceCount);
    
    // State functions
    static void Enable(uint32_t capability);
    static void Disable(uint32_t capability);
    static void BlendFunc(uint32_t sfactor, uint32_t dfactor);
    static void DepthFunc(uint32_t func);
    static void DepthMask(bool flag);
    static void ColorMask(bool r, bool g, bool b, bool a);
    static void Clear(uint32_t mask);
    static void ClearColor(float r, float g, float b, float a);
    static void PolygonOffset(float factor, float units);
    static void PolygonMode(uint32_t face, uint32_t mode);
    
    // Vertex array functions
    static void GenVertexArrays(uint32_t n, uint32_t* arrays);
    static void DeleteVertexArrays(uint32_t n, const uint32_t* arrays);
    static void BindVertexArray(uint32_t array);
    static void EnableVertexAttribArray(uint32_t index);
    static void DisableVertexAttribArray(uint32_t index);
    static void VertexAttribPointer(uint32_t index, int32_t size, uint32_t type, bool normalized, int32_t stride, const void* pointer);
    static void VertexAttribDivisor(uint32_t index, uint32_t divisor);
    
    // Buffer functions
    static void GenBuffers(uint32_t n, uint32_t* buffers);
    static void DeleteBuffers(uint32_t n, const uint32_t* buffers);
    static void BindBuffer(uint32_t target, uint32_t buffer);
    static void BufferData(uint32_t target, int32_t size, const void* data, uint32_t usage);
    static void BufferSubData(uint32_t target, int32_t offset, int32_t size, const void* data);
    
    // Texture functions
    static void GenTextures(uint32_t n, uint32_t* textures);
    static void DeleteTextures(uint32_t n, const uint32_t* textures);
    static void BindTexture(uint32_t target, uint32_t texture);
    static void ActiveTexture(uint32_t texture);
    static void TexImage2D(uint32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void* pixels);
    static void TexParameteri(uint32_t target, uint32_t pname, int32_t param);
    static void TexParameterf(uint32_t target, uint32_t pname, float param);
    static void GenerateMipmap(uint32_t target);
    static void PixelStorei(uint32_t pname, int32_t param);
    
    // Program functions
    static void UseProgram(uint32_t program);
    static int32_t GetUniformLocation(uint32_t program, const char* name);
    static void Uniform1i(int32_t location, int32_t v0);
    static void Uniform1f(int32_t location, float v0);
    static void Uniform2f(int32_t location, float v0, float v1);
    static void Uniform3f(int32_t location, float v0, float v1, float v2);
    static void Uniform4f(int32_t location, float v0, float v1, float v2, float v3);
    static void UniformMatrix2fv(int32_t location, int32_t count, bool transpose, const float* value);
    static void UniformMatrix3fv(int32_t location, int32_t count, bool transpose, const float* value);
    static void UniformMatrix4fv(int32_t location, int32_t count, bool transpose, const float* value);
    static void Uniform1fv(int32_t location, int32_t count, const float* value);
    static void Uniform1iv(int32_t location, int32_t count, const int32_t* value);
    static void Uniform2fv(int32_t location, int32_t count, const float* value);
    static void Uniform3fv(int32_t location, int32_t count, const float* value);
    static void Uniform4fv(int32_t location, int32_t count, const float* value);
    
    // Frame buffer functions
    static void GenFramebuffers(uint32_t n, uint32_t* framebuffers);
    static void DeleteFramebuffers(uint32_t n, const uint32_t* framebuffers);
    static void BindFramebuffer(uint32_t target, uint32_t framebuffer);
    static void FramebufferTexture2D(uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, int32_t level);
    static void BlitFramebuffer(int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, uint32_t mask, uint32_t filter);
    
    // Error checking
    static uint32_t GetError();
    static void GetIntegerv(uint32_t pname, int32_t* data);
    static void GetFloatv(uint32_t pname, float* data);
    
    // Constants
    static const uint32_t INVALID_HANDLE = UINT32_MAX;
    
    // OpenGL constants
    static const uint32_t VERTEX_SHADER = 0x8B31;
    static const uint32_t FRAGMENT_SHADER = 0x8B30;
    static const uint32_t TRIANGLES = 0x0004;
    static const uint32_t TRIANGLE_STRIP = 0x0005;
    static const uint32_t UNSIGNED_INT = 0x1405;
    static const uint32_t FLOAT = 0x1406;
    static const uint32_t ARRAY_BUFFER = 0x8892;
    static const uint32_t ELEMENT_ARRAY_BUFFER = 0x8893;
    static const uint32_t STATIC_DRAW = 0x88E4;
    static const uint32_t DYNAMIC_DRAW = 0x88E8;
    static const uint32_t STREAM_DRAW = 0x88E0;
    static const uint32_t TEXTURE_2D = 0x0DE1;
    static const uint32_t TEXTURE_CUBE_MAP = 0x8513;
    static const uint32_t TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
    static const uint32_t TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
    static const uint32_t TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
    static const uint32_t TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
    static const uint32_t TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
    static const uint32_t TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;
    static const uint32_t RGBA = 0x1908;
    static const uint32_t RGB = 0x1907;
    static const uint32_t UNSIGNED_BYTE = 0x1401;
    static const uint32_t LINEAR = 0x2601;
    static const uint32_t NEAREST = 0x2600;
    static const uint32_t LINEAR_MIPMAP_LINEAR = 0x2703;
    static const uint32_t CLAMP_TO_EDGE = 0x812F;
    static const uint32_t REPEAT = 0x2901;
    static const uint32_t TEXTURE_MIN_FILTER = 0x2801;
    static const uint32_t TEXTURE_MAG_FILTER = 0x2800;
    static const uint32_t TEXTURE_WRAP_S = 0x2802;
    static const uint32_t TEXTURE_WRAP_T = 0x2803;
    static const uint32_t TEXTURE_WRAP_R = 0x8072;
    static const uint32_t TEXTURE_COMPARE_MODE = 0x884C;
    static const uint32_t TEXTURE_COMPARE_FUNC = 0x884D;
    static const uint32_t COMPARE_REF_TO_TEXTURE = 0x884E;
    static const uint32_t LEQUAL = 0x0203;
    static const uint32_t NONE = 0x0000;
    static const uint32_t DEPTH_TEST = 0x0B71;
    static const uint32_t CULL_FACE = 0x0B44;
    static const uint32_t BLEND = 0x0BE2;
    static const uint32_t MULTISAMPLE = 0x809D;
    static const uint32_t POLYGON_OFFSET_FILL = 0x8037;
    static const uint32_t DITHER = 0x0BD0;
    static const uint32_t FRONT_AND_BACK = 0x0408;
    static const uint32_t LINE = 0x1B01;
    static const uint32_t FILL = 0x1B02;
    static const uint32_t SRC_ALPHA = 0x0302;
    static const uint32_t ONE_MINUS_SRC_ALPHA = 0x0303;
    static const uint32_t LESS = 0x0201;
    static const uint32_t COLOR_BUFFER_BIT = 0x00004000;
    static const uint32_t DEPTH_BUFFER_BIT = 0x00000100;
    static const uint32_t STENCIL_BUFFER_BIT = 0x00000400;
    static const uint32_t READ_FRAMEBUFFER = 0x8CA8;
    static const uint32_t DRAW_FRAMEBUFFER = 0x8CA9;
    static const uint32_t FRAMEBUFFER = 0x8D40;
    static const uint32_t COLOR_ATTACHMENT0 = 0x8CE0;
    static const uint32_t DEPTH_ATTACHMENT = 0x8D00;
    static const uint32_t STENCIL_ATTACHMENT = 0x8D00;
    static const uint32_t DEPTH_STENCIL_ATTACHMENT = 0x821A;
    static const uint32_t MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
    static const uint32_t MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0x84FF;
    static const uint32_t TEXTURE_MAX_ANISOTROPY_EXT = 0x84FE;
    static const uint32_t UNPACK_ALIGNMENT = 0x0CF5;
    static const uint32_t UNPACK_ROW_LENGTH = 0x0CF2;
    static const uint32_t ACTIVE_ATTRIBUTES = 0x8B89;
    static const uint32_t ACTIVE_UNIFORMS = 0x8B86;
    static const uint32_t COMPILE_STATUS = 0x8B81;
    static const uint32_t LINK_STATUS = 0x8B82;
    static const uint32_t INFO_LOG_LENGTH = 0x8B84;
#undef NO_ERROR
    static const uint32_t NO_ERROR = 0x0000;
    static const uint32_t TEXTURE0 = 0x84C0;
    static const uint32_t TEXTURE1 = 0x84C1;
    static const uint32_t TEXTURE2 = 0x84C2;
    static const uint32_t TEXTURE3 = 0x84C3;
    static const uint32_t TEXTURE4 = 0x84C4;
    static const uint32_t TEXTURE5 = 0x84C5;
    static const uint32_t TEXTURE6 = 0x84C6;
    static const uint32_t TEXTURE7 = 0x84C7;
    static const uint32_t TEXTURE8 = 0x84C8;
    static const uint32_t TEXTURE9 = 0x84C9;
    static const uint32_t TEXTURE10 = 0x84CA;
    static const uint32_t TEXTURE11 = 0x84CB;
    static const uint32_t TEXTURE12 = 0x84CC;
    static const uint32_t TEXTURE13 = 0x84CD;
    static const uint32_t TEXTURE14 = 0x84CE;
    static const uint32_t TEXTURE15 = 0x84CF;
    static const uint32_t TEXTURE16 = 0x84D0;
    static const uint32_t TEXTURE17 = 0x84D1;
    static const uint32_t TEXTURE18 = 0x84D2;
    static const uint32_t TEXTURE19 = 0x84D3;
    static const uint32_t TEXTURE20 = 0x84D4;
    static const uint32_t TEXTURE21 = 0x84D5;
    static const uint32_t TEXTURE22 = 0x84D6;
    static const uint32_t TEXTURE23 = 0x84D7;
    static const uint32_t TEXTURE24 = 0x84D8;
    static const uint32_t TEXTURE25 = 0x84D9;
    static const uint32_t TEXTURE26 = 0x84DA;
    static const uint32_t TEXTURE27 = 0x84DB;
    static const uint32_t TEXTURE28 = 0x84DC;
    static const uint32_t TEXTURE29 = 0x84DD;
    static const uint32_t TEXTURE30 = 0x84DE;
    static const uint32_t TEXTURE31 = 0x84DF;
    
private:
    static Backend s_currentBackend;
    static bool s_initialized;
    static SDL_Window* s_window;
};
