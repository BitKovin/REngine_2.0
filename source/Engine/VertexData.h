#pragma once

#include "glm.h"
#include "gl.h"
#include <vector>

#include "EObject.hpp"

#define OFFSET_OF(type, member) ((void*)offsetof(type, member))

class VertexDeclaration {
public:
    struct Element {
        GLuint index;
        GLint componentCount;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void* offset;
        GLuint divisor;
    };

    VertexDeclaration(std::initializer_list<Element> elements) : m_elements(elements) {}
    const std::vector<Element>& GetElements() const { return m_elements; }

private:
    std::vector<Element> m_elements;
};

class VertexBuffer : public EObject {
public:
    template<typename T>
    VertexBuffer(const std::vector<T>& vertices, const VertexDeclaration& declaration, GLenum usage = GL_STATIC_DRAW)
        : m_declaration(declaration), m_vertexCount(vertices.size()) {
        glGenBuffers(1, &m_id);
        Bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), usage);
    }

    ~VertexBuffer() { glDeleteBuffers(1, &m_id); }

    void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_id); }
    static void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    const VertexDeclaration& GetDeclaration() const { return m_declaration; }
    size_t GetVertexCount() const { return m_vertexCount; }

    // New method to update buffer data
    template<typename T>
    void UpdateData(const std::vector<T>& data, size_t offset = 0, GLenum usage = GL_DYNAMIC_DRAW) {
        Bind();
        // If the new data size differs from the currently allocated one, reallocate the buffer.
        if (data.size() != m_vertexCount) {
            m_vertexCount = data.size();
            glBufferData(GL_ARRAY_BUFFER, m_vertexCount * sizeof(T), data.data(), usage);
        }
        else {
            glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(T), data.size() * sizeof(T), data.data());
        }
    }

private:
    GLuint m_id;
    VertexDeclaration m_declaration;
    size_t m_vertexCount;
};

class IndexBuffer : public EObject {
public:
    IndexBuffer(const std::vector<GLuint>& indices, GLenum usage = GL_STATIC_DRAW)
        : m_indexCount(indices.size()) {
        glGenBuffers(1, &m_id);
        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), usage);
    }

    ~IndexBuffer() { glDeleteBuffers(1, &m_id); }
    void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id); }
    static void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
    void UpdateData(const std::vector<GLuint>& data, GLenum usage = GL_STREAM_DRAW) {
        Bind();
        m_indexCount = data.size();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(GLuint), data.data(), usage);
    }
    size_t GetIndexCount() const { return m_indexCount; }

private:
    GLuint m_id;
    size_t m_indexCount;
};

class VertexArrayObject : public EObject {
public:
    int IndexCount = 0;
    VertexBuffer* vertexBuffer = nullptr;
    IndexBuffer* indexBuffer = nullptr;
    VertexBuffer* instanceBuffer = nullptr;

    VertexArrayObject(VertexBuffer& vb, IndexBuffer& ib, VertexBuffer* instanceBuf = nullptr) {
        glGenVertexArrays(1, &m_id);
        glBindVertexArray(m_id);

        IndexCount = ib.GetIndexCount();
        vertexBuffer = &vb;
        indexBuffer = &ib;
        instanceBuffer = instanceBuf;

        vb.Bind();
        ib.Bind();
        const auto& vertexElements = vb.GetDeclaration().GetElements();
        for (const auto& element : vertexElements) {
            glEnableVertexAttribArray(element.index);
            const bool isIntegerType =
                element.type == GL_INT ||
                element.type == GL_UNSIGNED_INT ||
                element.type == GL_BYTE ||
                element.type == GL_UNSIGNED_BYTE ||
                element.type == GL_SHORT ||
                element.type == GL_UNSIGNED_SHORT;

            if (isIntegerType && !element.normalized) {
                glVertexAttribIPointer(
                    element.index,
                    element.componentCount,
                    element.type,
                    element.stride,
                    element.offset
                );
            }
            else {
                glVertexAttribPointer(
                    element.index,
                    element.componentCount,
                    element.type,
                    element.normalized,
                    element.stride,
                    element.offset
                );
            }
            if (element.divisor > 0) {
                glVertexAttribDivisor(element.index, element.divisor);
            }
        }

        if (instanceBuffer) {
            instanceBuffer->Bind();
            const auto& instanceElements = instanceBuffer->GetDeclaration().GetElements();
            for (const auto& element : instanceElements) {
                glEnableVertexAttribArray(element.index);
                glVertexAttribPointer(
                    element.index,
                    element.componentCount,
                    element.type,
                    element.normalized,
                    element.stride,
                    element.offset
                );
                glVertexAttribDivisor(element.index, element.divisor);
            }
        }

        glBindVertexArray(0);
        VertexBuffer::Unbind();
        IndexBuffer::Unbind();
    }

    ~VertexArrayObject() { glDeleteVertexArrays(1, &m_id); }
    void Bind() const { glBindVertexArray(m_id); }
    static void Unbind() { glBindVertexArray(0); }
    bool IsInstanced() const { return instanceBuffer != nullptr; }
    size_t GetInstanceCount() const { return instanceBuffer ? instanceBuffer->GetVertexCount() : 0; }

private:
    GLuint m_id;
};

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
            {0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Position), 0},
            {1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Normal), 0},
            {2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, TextureCoordinate), 0},
            {3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Tangent), 0},
            {4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BiTangent), 0},
            {5, 4, GL_INT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BlendIndices), 0},
            {6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BlendWeights), 0},
            {7, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, SmoothNormal), 0},
            {8, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Color), 0},
            { 9, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, ShadowMapCoords), 0 }
            });
    }
};

struct InstanceData {
    glm::mat4 ModelMatrix;
    glm::vec4 Color;

    static VertexDeclaration Declaration() {
        return VertexDeclaration({
            {10, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)0, 1},
            {11, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(sizeof(glm::vec4)), 1},
            {12, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(2 * sizeof(glm::vec4)), 1},
            {13, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(3 * sizeof(glm::vec4)), 1},
            {14, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(4 * sizeof(glm::vec4)), 1}
            });
    }
};