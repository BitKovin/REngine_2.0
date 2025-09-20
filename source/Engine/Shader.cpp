#include "Shader.hpp"

#include "AssetRegistry.h"

#include <regex>
#include <string>

bool Shader::Reload()
{
    // 1) obtain new source (file if available, otherwise stored source)
    std::string newSource;
    if (!filePath.empty())
        newSource = FileSystemEngine::ReadFile(filePath);
    else
        newSource = shaderCode;

    if (newSource.empty())
    {
        Logger::Log("Shader::Reload: source is empty for " + filePath);
        return false;
    }

    // 2) compile new shader object
    uint32_t shaderTypeFlag = (shaderType == VertexShader) ? RenderInterface::VERTEX_SHADER : RenderInterface::FRAGMENT_SHADER;
    
    // Use GLSL compilation directly
    uint32_t newShader = RenderInterface::CreateShaderFromGLSL(newSource.c_str(), shaderTypeFlag);
    
    if (newShader == RenderInterface::INVALID_HANDLE)
    {
        Logger::Log("Shader compilation failed during reload for shader type: " + std::to_string(shaderType));
        Logger::Log(newSource);
        return false;
    }

    // 3) swap attempt across all programs that reference this shader
    uint32_t oldShader = shaderHandle; // may be INVALID_HANDLE if none previously
    std::vector<ShaderProgram*> modifiedPrograms; // programs we've successfully switched to newShader

    for (ShaderProgram* prog : attachedPrograms)
    {
        // For bgfx, we need to recreate the program with the new shader
        // This is a simplified approach - in practice, you'd want to handle this more carefully
        
        // Destroy the old program
        if (prog->program != RenderInterface::INVALID_HANDLE)
        {
            RenderInterface::DestroyProgram(prog->program);
            prog->program = RenderInterface::INVALID_HANDLE;
        }
        
        // Recreate the program with the new shader
        // This is a simplified approach - in practice, you'd want to find the other shader
        // and recreate the program properly
        prog->LinkProgram();
        
        if (prog->program == RenderInterface::INVALID_HANDLE)
        {
            std::string shaderKind = (shaderType == VertexShader) ? "Vertex" : "Fragment";
            Logger::Log("Shader program linking failed while reloading shader: " + shaderKind);
            
            // ROLLBACK: restore previously modified programs to old shader
            for (ShaderProgram* mprog : modifiedPrograms)
            {
                // For bgfx, we'd need to recreate the program with the old shader
                // This is a simplified approach
                mprog->LinkProgram();
                mprog->FillAttributes();
                mprog->CacheUniformLocations();
            }

            // Also rollback the current program
            // For bgfx, we'd need to recreate the program with the old shader
            // This is a simplified approach
            prog->LinkProgram();
            prog->FillAttributes();
            prog->CacheUniformLocations();

            // delete the new shader object and abort
            RenderInterface::DestroyShader(newShader);
            Logger::Log("Shader::Reload aborted — link error(s). Keeping old shader.");
            return false;
        }

        // success for this program -> update caches & record it
        prog->FillAttributes();
        prog->CacheUniformLocations();
        prog->textureBindings = prog->ParseAllTextureBindings();
        modifiedPrograms.push_back(prog);
    }

    // 4) All programs linked successfully with newShader — commit the change
    shaderHandle = newShader;
    shaderCode = newSource;

    // delete old shader if present
    if (oldShader != RenderInterface::INVALID_HANDLE)
        RenderInterface::DestroyShader(oldShader);

    Logger::Log("Shader::Reload succeeded for " + filePath);
    return true;
}

std::unordered_map<std::string, std::string> ShaderProgram::ParseTextureBindings(const std::string& shaderCode)
{
    std::unordered_map<std::string, std::string> result;

    // Regex: match `uniform sampler2D <name>; // @texture <path>`
    std::regex re(R"(uniform\s+sampler2D\s+(\w+)\s*;.*@texture\s+([^\s]+))");

    std::smatch match;
    std::string::const_iterator searchStart(shaderCode.cbegin());

    while (std::regex_search(searchStart, shaderCode.cend(), match, re))
    {
        std::string uniformName = match[1].str();
        std::string texturePath = match[2].str();
        result[uniformName] = texturePath;

        searchStart = match.suffix().first;
    }

    return result;
}

std::unordered_map<std::string, std::string> ShaderProgram::ParseAllTextureBindings() const
{
    std::unordered_map<std::string, std::string> result;

    // Regex matches: uniform sampler2D myTex; // @texture path/to/file.png
    std::regex re(R"(uniform\s+(?:sampler2D|samplerCube)\s+(\w+)\s*;.*@texture\s+([^\s]+))");

    for (const Shader* s : attachedShaders)
    {
        if (!s) continue;

        const std::string& code = s->shaderCode;

        std::smatch match;
        std::string::const_iterator searchStart(code.cbegin());

        while (std::regex_search(searchStart, code.cend(), match, re))
        {
            std::string uniformName = match[1].str();
            std::string texturePath = match[2].str();

            // overwrite if duplicate uniform name across shaders
            result[uniformName] = texturePath;

            searchStart = match.suffix().first;
        }
    }

    return result;
}

void ShaderProgram::ParseShaders()
{

    // For each attached shader, parse bindings
    for (Shader* s : attachedShaders)
    {
        auto parsed = ParseTextureBindings(s->shaderCode);
        for (auto& kv : parsed)
        {
            textureBindings[kv.first] = kv.second;
        }
    }

}

void ShaderProgram::ApplyTextureBindings()
{

    for (const auto& pair : textureBindings)
    {
        const std::string& uniformName = pair.first;
        const std::string& path = pair.second;

        Texture* tex = AssetRegistry::GetTextureFromFile(path); 
        if (tex)
            SetTexture(uniformName, tex);
    }

}
