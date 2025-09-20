#pragma once

#include "glm.h"
#include <glm/glm.hpp>
#include "Renderer/RHI/RenderInterface.h"
#include <vector>

#include "EObject.hpp"

#define OFFSET_OF(type, member) ((void*)offsetof(type, member))

class VertexDeclaration {
public:
    struct Element {
        uint32_t index;
        int32_t componentCount;
        uint32_t type;
        bool normalized;
        int32_t stride;
        const void* offset;
        uint32_t divisor;
    };

    VertexDeclaration(std::initializer_list<Element> elements) : m_elements(elements) {}
    const std::vector<Element>& GetElements() const { return m_elements; }

private:
    std::vector<Element> m_elements;
};

class VertexBuffer : public EObject {
public:
    template<typename T>
    VertexBuffer(const std::vector<T>& vertices, const VertexDeclaration& declaration, uint32_t usage = RenderInterface::STATIC_DRAW)
        : m_declaration(declaration), m_vertexCount(vertices.size()) {
        // Create bgfx memory from vertex data
        void* mem = RenderInterface::Alloc(vertices.size() * sizeof(T));
        memcpy(mem, vertices.data(), vertices.size() * sizeof(T));
        
        // Create vertex buffer with bgfx
        m_id = RenderInterface::CreateVertexBuffer(mem, vertices.size() * sizeof(T), &declaration, 0);
        
        RenderInterface::Free(mem);
    }

    ~VertexBuffer() { 
        if (m_id != RenderInterface::INVALID_HANDLE)
            RenderInterface::DestroyVertexBuffer(m_id); 
    }

    void Bind() const { 
        // For bgfx, binding is handled differently
        // This is a placeholder for compatibility
    }
    static void Unbind() { 
        // For bgfx, unbinding is handled differently
        // This is a placeholder for compatibility
    }

    const VertexDeclaration& GetDeclaration() const { return m_declaration; }
    size_t GetVertexCount() const { return m_vertexCount; }

    // New method to update buffer data
    template<typename T>
    void UpdateData(const std::vector<T>& data, size_t offset = 0, uint32_t usage = RenderInterface::DYNAMIC_DRAW) {
        // For bgfx, we need to recreate the buffer with new data
        // This is a simplified approach - in practice, you'd want to handle this more efficiently
        if (m_id != RenderInterface::INVALID_HANDLE) {
            RenderInterface::DestroyVertexBuffer(m_id);
        }
        
        // Create new buffer with updated data
        void* mem = RenderInterface::Alloc(data.size() * sizeof(T));
        memcpy(mem, data.data(), data.size() * sizeof(T));
        
        m_id = RenderInterface::CreateVertexBuffer(mem, data.size() * sizeof(T), &m_declaration, 0);
        m_vertexCount = data.size();
        
        RenderInterface::Free(mem);
    }

private:
    uint32_t m_id;
    VertexDeclaration m_declaration;
    size_t m_vertexCount;
};

class IndexBuffer : public EObject {
public:
    IndexBuffer(const std::vector<uint32_t>& indices, uint32_t usage = RenderInterface::STATIC_DRAW)
        : m_indexCount(indices.size()) {
        // Create bgfx memory from index data
        void* mem = RenderInterface::Alloc(indices.size() * sizeof(uint32_t));
        memcpy(mem, indices.data(), indices.size() * sizeof(uint32_t));
        
        // Create index buffer with bgfx
        m_id = RenderInterface::CreateIndexBuffer(mem, indices.size() * sizeof(uint32_t), 0);
        
        RenderInterface::Free(mem);
    }

    ~IndexBuffer() { 
        if (m_id != RenderInterface::INVALID_HANDLE)
            RenderInterface::DestroyIndexBuffer(m_id); 
    }
    void Bind() const { 
        // For bgfx, binding is handled differently
        // This is a placeholder for compatibility
    }
    static void Unbind() { 
        // For bgfx, unbinding is handled differently
        // This is a placeholder for compatibility
    }
    void UpdateData(const std::vector<uint32_t>& data, uint32_t usage = RenderInterface::STREAM_DRAW) {
        // For bgfx, we need to recreate the buffer with new data
        if (m_id != RenderInterface::INVALID_HANDLE) {
            RenderInterface::DestroyIndexBuffer(m_id);
        }
        
        // Create new buffer with updated data
        void* mem = RenderInterface::Alloc(data.size() * sizeof(uint32_t));
        memcpy(mem, data.data(), data.size() * sizeof(uint32_t));
        
        m_id = RenderInterface::CreateIndexBuffer(mem, data.size() * sizeof(uint32_t), 0);
        m_indexCount = data.size();
        
        RenderInterface::Free(mem);
    }
    size_t GetIndexCount() const { return m_indexCount; }

private:
    uint32_t m_id;
    size_t m_indexCount;
};

// ------------------------------------------------------------
// VertexArrayObject: real VAO on non-GLES2, emulated on GLES2
// ------------------------------------------------------------
#ifndef GL_ES_2

// --- bgfx VAO implementation ---
class VertexArrayObject : public EObject {
public:
    int IndexCount = 0;
    VertexBuffer* vertexBuffer = nullptr;
    IndexBuffer* indexBuffer = nullptr;
    VertexBuffer* instanceBuffer = nullptr;

    VertexArrayObject(VertexBuffer& vb, IndexBuffer& ib, VertexBuffer* instanceBuf = nullptr) {
        // For bgfx, we don't need to create a VAO as it handles vertex layouts differently
        // We just store references to the buffers
        IndexCount = ib.GetIndexCount();
        vertexBuffer = &vb;
        indexBuffer = &ib;
        instanceBuffer = instanceBuf;
        
        // For bgfx, the vertex layout is defined when creating the vertex buffer
        // and the VAO concept is handled internally by bgfx
    }

    ~VertexArrayObject() { 
        // For bgfx, we don't need to delete anything as it's handled internally
    }
    void Bind() const { 
        // For bgfx, binding is handled differently
        // This is a placeholder for compatibility
    }
    static void Unbind() { 
        // For bgfx, unbinding is handled differently
        // This is a placeholder for compatibility
    }
    bool IsInstanced() const { return instanceBuffer != nullptr; }
    size_t GetInstanceCount() const { return instanceBuffer ? instanceBuffer->GetVertexCount() : 0; }

private:
    // For bgfx, we don't need to store an ID as it's handled internally
};

#else // GL_ES_2

// --- bgfx fallback: same as desktop path ---
// For bgfx, we use the same implementation as the desktop path
// since bgfx handles all the platform differences internally
class VertexArrayObject : public EObject {
public:
    int IndexCount = 0;
    VertexBuffer* vertexBuffer = nullptr;
    IndexBuffer* indexBuffer = nullptr;
    VertexBuffer* instanceBuffer = nullptr;

    VertexArrayObject(VertexBuffer& vb, IndexBuffer& ib, VertexBuffer* instanceBuf = nullptr) {
        // For bgfx, we don't need to create a VAO as it handles vertex layouts differently
        // We just store references to the buffers
        IndexCount = ib.GetIndexCount();
        vertexBuffer = &vb;
        indexBuffer = &ib;
        instanceBuffer = instanceBuf;
        
        // For bgfx, the vertex layout is defined when creating the vertex buffer
        // and the VAO concept is handled internally by bgfx
    }

    ~VertexArrayObject() { 
        // For bgfx, we don't need to delete anything as it's handled internally
    }

    // Bind: set up attributes (emulates binding a VAO)
    void Bind() const {
        // For bgfx, binding is handled differently
        // This is a placeholder for compatibility
    }

    // Unbind: disable attributes enabled by the last Bind() and unbind buffers
    static void Unbind() {
        // For bgfx, unbinding is handled differently
        // This is a placeholder for compatibility
    }

    bool IsInstanced() const {
        return instanceBuffer != nullptr;
    }

    size_t GetInstanceCount() const {
        return instanceBuffer ? instanceBuffer->GetVertexCount() : 0;
    }

private:
    // For bgfx, we don't need to store these as it's handled internally
};

#endif // GL_ES_2

struct VertexData {
    glm::vec3 Position = glm::vec3();
    glm::vec3 Normal = glm::vec3();
    glm::vec2 TextureCoordinate = glm::vec2();
    glm::vec3 Tangent = glm::vec3();
    glm::vec3 BiTangent = glm::vec3();
    int BlendIndices[4] = { 0, 0, 0, 0 };
    glm::vec4 BlendWeights = glm::vec4();
    glm::vec3 SmoothNormal = glm::vec3();
    glm::vec4 Color = glm::vec4(1);
    glm::vec2 ShadowMapCoords = glm::vec4(1);

    static VertexDeclaration Declaration() {
        return VertexDeclaration({
            {0, 3, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, Position), 0},
            {1, 3, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, Normal), 0},
            {2, 2, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, TextureCoordinate), 0},
            {3, 3, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, Tangent), 0},
            {4, 3, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, BiTangent), 0},
            {5, 4, RenderInterface::UNSIGNED_INT, false, sizeof(VertexData), OFFSET_OF(VertexData, BlendIndices), 0},
            {6, 4, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, BlendWeights), 0},
            {7, 3, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, SmoothNormal), 0},
            {8, 4, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, Color), 0},
            { 9, 2, RenderInterface::FLOAT, false, sizeof(VertexData), OFFSET_OF(VertexData, ShadowMapCoords), 0 }
            });
    }
};

struct InstanceData {
    glm::mat4 ModelMatrix;
    glm::vec4 Color;

    static VertexDeclaration Declaration() {
        return VertexDeclaration({
            {10, 4, RenderInterface::FLOAT, false, sizeof(InstanceData), (void*)0, 1},
            {11, 4, RenderInterface::FLOAT, false, sizeof(InstanceData), (void*)(sizeof(glm::vec4)), 1},
            {12, 4, RenderInterface::FLOAT, false, sizeof(InstanceData), (void*)(2 * sizeof(glm::vec4)), 1},
            {13, 4, RenderInterface::FLOAT, false, sizeof(InstanceData), (void*)(3 * sizeof(glm::vec4)), 1},
            {14, 4, RenderInterface::FLOAT, false, sizeof(InstanceData), (void*)(4 * sizeof(glm::vec4)), 1}
            });
    }
};