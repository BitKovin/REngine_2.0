#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Logger.hpp"
#include "Renderer/RHI/RenderInterface.h"
#include "glm.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.hpp"
#include "FileSystem/FileSystem.h"
#include "malloc_override.h"

using namespace std;

enum ShaderType
{
    VertexShader,
    PixelShader
};

// Forward declare so Shader can reference ShaderProgram and vice-versa
class ShaderProgram;

// Struct for storing shader attribute information.
struct ShaderAttribute
{
    std::string name;  // Attribute name.
    uint32_t type = RenderInterface::INVALID_HANDLE;       // Data type (e.g., FLOAT, INT).
    int32_t size = -1;        // Size (number of components, or array size).
    int32_t location = -1;    // Location within the shader program.
};

class Shader
{
public:
    Shader() {}

    uint32_t shaderHandle = RenderInterface::INVALID_HANDLE;
    std::string shaderCode = "";
    std::string filePath = "";
    ShaderType shaderType = ShaderType::PixelShader;

    // List of programs that currently have this shader attached (registered via ShaderProgram::AttachShader).
    std::vector<ShaderProgram*> attachedPrograms;

    // Creates a Shader object from source code.
    static Shader* FromCode(const char* code, ShaderType shaderType, bool autoCompile = true)
    {
        Shader* output = new Shader();
        uint32_t shaderTypeFlag = (shaderType == VertexShader) ? RenderInterface::VERTEX_SHADER : RenderInterface::FRAGMENT_SHADER;

        output->shaderCode = code;
        output->shaderType = shaderType;

        if (autoCompile)
            output->CompileShader();

        return output;
    }

    static Shader* FromFile(const char* filePath, ShaderType shaderType, bool autoCompile = true)
    {
        Shader* output = FromCode(FileSystemEngine::ReadFile(filePath).c_str(), shaderType, autoCompile);
        output->filePath = filePath;

        return output;
    }

    // Compiles the shader and logs compile errors if any.
    void CompileShader()
    {
        uint32_t shaderTypeFlag = (shaderType == VertexShader) ? RenderInterface::VERTEX_SHADER : RenderInterface::FRAGMENT_SHADER;
        
        // Use GLSL compilation directly
        shaderHandle = RenderInterface::CreateShaderFromGLSL(shaderCode.c_str(), shaderTypeFlag);
        
        if (shaderHandle == RenderInterface::INVALID_HANDLE)
        {
            Logger::Log("Shader compilation failed for shader type: " + std::to_string(shaderType));
            Logger::Log(shaderCode);
        }
    }
    

    // Register/unregister programs that use this shader
    void RegisterProgram(ShaderProgram* prog)
    {
        if (std::find(attachedPrograms.begin(), attachedPrograms.end(), prog) == attachedPrograms.end())
            attachedPrograms.push_back(prog);
    }

    void UnregisterProgram(ShaderProgram* prog)
    {
        auto it = std::find(attachedPrograms.begin(), attachedPrograms.end(), prog);
        if (it != attachedPrograms.end())
            attachedPrograms.erase(it);
    }

    // Hot reloads shader source (from file if filePath is set, otherwise uses stored shaderCode).
    // Returns true if reload succeeded and programs were relinked, false if compilation or linking failed
    // (in case of failure the old shader stays attached).
    bool Reload();
};

class ShaderProgram
{

private:
    std::unordered_map<std::string, uint32_t> m_textureUnits;
    uint32_t m_currentUnit = 0;
    uint32_t m_maxTextureUnits = 16; // Will be initialized from render interface



public:
    uint32_t program;
    std::vector<ShaderAttribute> attributes;  // Stores shader attributes.
    std::unordered_map<std::string, int32_t> uniformLocations; // Cache for uniform locations.

    std::unordered_map<std::string, std::string> textureBindings; 

    // Keep track of Shader* that are attached to this program (so we can unregister on destruction).
    std::vector<Shader*> attachedShaders;

    string name;

    bool AllowMissingUniforms = true; // keep your original semantics (you had 'true' earlier)

    ShaderProgram() {
        RenderInterface::GetIntegerv(RenderInterface::MAX_COMBINED_TEXTURE_IMAGE_UNITS, (int32_t*)&m_maxTextureUnits);
        program = RenderInterface::INVALID_HANDLE;
    }

    ~ShaderProgram()
    {
        // unregister this program from attached shaders
        for (auto* s : attachedShaders)
        {
            if (s) s->UnregisterProgram(this);
        }

        if (program != RenderInterface::INVALID_HANDLE)
            RenderInterface::DestroyProgram(program);
    }

    // Attaches a compiled shader to the program.
    ShaderProgram* AttachShader(Shader* shader)
    {
        if (!shader) return this;

        // For bgfx, we need to create the program when we have both vertex and fragment shaders
        // This is a simplified approach - in practice, you'd want to handle this more carefully
        if (program == RenderInterface::INVALID_HANDLE)
        {
            // We'll create the program when we have both shaders
            // For now, just store the shader reference
        }

        // remember locally
        if (std::find(attachedShaders.begin(), attachedShaders.end(), shader) == attachedShaders.end())
            attachedShaders.push_back(shader);

        // register this program with the shader so Shader::Reload can find all programs using it
        shader->RegisterProgram(this);

        return this;
    }

    // Links the program.
    ShaderProgram* LinkProgram()
    {
        // Find vertex and fragment shaders
        Shader* vertexShader = nullptr;
        Shader* fragmentShader = nullptr;
        
        for (auto* shader : attachedShaders)
        {
            if (shader->shaderType == VertexShader)
                vertexShader = shader;
            else if (shader->shaderType == PixelShader)
                fragmentShader = shader;
        }
        
        if (vertexShader && fragmentShader)
        {
            program = RenderInterface::CreateProgram(vertexShader->shaderHandle, fragmentShader->shaderHandle, false);
            
            if (program == RenderInterface::INVALID_HANDLE)
            {
                Logger::Log("Shader program linking failed");
            }
        }
        else
        {
            Logger::Log("Shader program linking failed: missing vertex or fragment shader");
        }

        FillAttributes();
        CacheUniformLocations();

        textureBindings = ParseAllTextureBindings();

        return this;
    }

    // Activates the program.
    void UseProgram()
    {
        // For bgfx, we don't need to unbind textures as it handles this automatically
        // The program will be used when we call RenderInterface::Submit
        
        ApplyTextureBindings();
    }

    // Fills the attributes vector by querying the linked program.
    void FillAttributes()
    {
        attributes.clear();
        // For bgfx, we don't need to query attributes as they're defined in the vertex layout
        // This is a simplified approach - in practice, you'd want to parse the shader code
        // or use bgfx's reflection capabilities
    }

    // Caches uniform locations to avoid redundant uniform location calls.
    void CacheUniformLocations()
    {
        uniformLocations.clear();
        // For bgfx, we don't need to query uniform locations as they're handled differently
        // This is a simplified approach - in practice, you'd want to parse the shader code
        // or use bgfx's reflection capabilities
    }

    // Retrieves a cached uniform location.
    int32_t GetUniformLocation(const std::string& name)
    {
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
            return it->second;

        int32_t location = RenderInterface::GetUniformLocation(program, name.c_str());

        uniformLocations[name] = location;

        if (location >= 0)
            return location;

        if (AllowMissingUniforms == false)
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");

        return -1;
    }

    void SetTexture(const std::string& name, uint32_t texture) {
        int32_t location = GetUniformLocation(name);
        if (location == -1) return;

        // Find or assign texture unit
        auto it = m_textureUnits.find(name);
        if (it == m_textureUnits.end()) {
            if (m_currentUnit >= m_maxTextureUnits) {
                Logger::Log("Texture unit overflow! Maximum: " +
                    std::to_string(m_maxTextureUnits));
                return;
            }
            m_textureUnits[name] = m_currentUnit++;
        }

        uint32_t unit = m_textureUnits[name];

        // Bind texture and update uniform
        RenderInterface::ActiveTexture(RenderInterface::TEXTURE0 + unit);
        RenderInterface::BindTexture(RenderInterface::TEXTURE_2D, texture);
        RenderInterface::Uniform1i(location, unit);
    }

    void SetCubemapTexture(const std::string& name, uint32_t texture) {
        int32_t location = GetUniformLocation(name);
        if (location == -1) return;

        // Find or assign texture unit
        auto it = m_textureUnits.find(name);
        if (it == m_textureUnits.end()) {
            if (m_currentUnit >= m_maxTextureUnits) {
                Logger::Log("Texture unit overflow! Maximum: " +
                    std::to_string(m_maxTextureUnits));
                return;
            }
            m_textureUnits[name] = m_currentUnit++;
        }

        uint32_t unit = m_textureUnits[name];

        // Bind cubemap texture and update uniform
        RenderInterface::ActiveTexture(RenderInterface::TEXTURE0 + unit);
        RenderInterface::BindTexture(RenderInterface::TEXTURE_CUBE_MAP, texture);
        RenderInterface::Uniform1i(location, unit);
    }

    void SetTexture(const std::string& name, Texture* texture) {
        int32_t location = GetUniformLocation(name);
        if (location == -1) return;

        // Find or assign texture unit
        auto it = m_textureUnits.find(name);
        if (it == m_textureUnits.end()) {
            if (m_currentUnit >= m_maxTextureUnits) {
                Logger::Log("Texture unit overflow! Maximum: " +
                    std::to_string(m_maxTextureUnits));
                return;
            }
            m_textureUnits[name] = m_currentUnit++;
        }

        uint32_t unit = m_textureUnits[name];

        // Bind texture and update uniform
        RenderInterface::ActiveTexture(RenderInterface::TEXTURE0 + unit);
        RenderInterface::BindTexture(RenderInterface::TEXTURE_2D, texture == nullptr? 0 : texture->getID());
        RenderInterface::Uniform1i(location, unit);
    }


    // === Uniform setting functions with cached locations ===
    
        // Set uniform integer
    void SetUniform(const std::string& name, int value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::Uniform1i(location, value);
    }

    // Set uniform float
    void SetUniform(const std::string& name, float value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::Uniform1f(location, value);
    }

    // Set uniform vec2
    void SetUniform(const std::string& name, const glm::vec2& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::Uniform2f(location, value.x, value.y);
    }

    // Set uniform vec3
    void SetUniform(const std::string& name, const glm::vec3& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::Uniform3f(location, value.x, value.y, value.z);
    }

    // Set uniform vec4
    void SetUniform(const std::string& name, const glm::vec4& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::Uniform4f(location, value.x, value.y, value.z, value.w);
    }

    // Set uniform mat2
    void SetUniform(const std::string& name, const glm::mat2& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::UniformMatrix2fv(location, 1, false, glm::value_ptr(value));
    }

    // Set uniform mat3
    void SetUniform(const std::string& name, const glm::mat3& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::UniformMatrix3fv(location, 1, false, glm::value_ptr(value));
    }

    // Set uniform mat4
    void SetUniform(const std::string& name, const glm::mat4& value)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1) RenderInterface::UniformMatrix4fv(location, 1, false, glm::value_ptr(value));
    }

    // Set uniform array of floats
    void SetUniform(const std::string& name, const std::vector<float>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::Uniform1fv(location, static_cast<int32_t>(values.size()), values.data());
    }

    // Set uniform array of ints
    void SetUniform(const std::string& name, const std::vector<int>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::Uniform1iv(location, static_cast<int32_t>(values.size()), values.data());
    }

    // Set uniform array of vec2
    void SetUniform(const std::string& name, const std::vector<glm::vec2>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::Uniform2fv(location, static_cast<int32_t>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of vec3
    void SetUniform(const std::string& name, const std::vector<glm::vec3>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::Uniform3fv(location, static_cast<int32_t>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of vec4
    void SetUniform(const std::string& name, const std::vector<glm::vec4>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::Uniform4fv(location, static_cast<int32_t>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of mat2
    void SetUniform(const std::string& name, const std::vector<glm::mat2>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::UniformMatrix2fv(location,
                static_cast<int32_t>(values.size()),
                false,
                glm::value_ptr(values[0]));
    }

    // Set uniform array of mat3
    void SetUniform(const std::string& name, const std::vector<glm::mat3>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::UniformMatrix3fv(location,
                static_cast<int32_t>(values.size()),
                false,
                glm::value_ptr(values[0]));
    }

    // Set uniform array of mat4 
    void SetUniform(const std::string& name, const std::vector<glm::mat4>& values)
    {
        int32_t location = GetUniformLocation(name);
        if (location != -1)
            RenderInterface::UniformMatrix4fv(location,
                static_cast<int32_t>(values.size()),
                false,
                glm::value_ptr(values[0]));
    }

    

    static std::unordered_map<std::string, std::string> ParseTextureBindings(const std::string& shaderCode);

    std::unordered_map<std::string, std::string> ParseAllTextureBindings() const;

    void ParseShaders();

    void ApplyTextureBindings();

};
