#include "Renderer/RHI/RenderInterface.h"
#include "Renderer/RHI/Bgfx/bgfx_wrapper.h"
#include <iostream>

int main() {
    // Test basic bgfx functionality
    std::cout << "Testing bgfx implementation..." << std::endl;
    
    // Test constants
    std::cout << "INVALID_HANDLE: " << RenderInterface::INVALID_HANDLE << std::endl;
    std::cout << "VERTEX_SHADER: " << RenderInterface::VERTEX_SHADER << std::endl;
    std::cout << "FRAGMENT_SHADER: " << RenderInterface::FRAGMENT_SHADER << std::endl;
    
    // Test shader creation (this will fail without proper initialization, but we can test the interface)
    const char* vertexShader = "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main() { gl_Position = vec4(aPos, 1.0); }";
    uint32_t shader = RenderInterface::CreateShaderFromGLSL(vertexShader, RenderInterface::VERTEX_SHADER);
    
    if (shader == RenderInterface::INVALID_HANDLE) {
        std::cout << "Shader creation failed (expected without proper initialization)" << std::endl;
    } else {
        std::cout << "Shader created successfully: " << shader << std::endl;
        RenderInterface::DestroyShader(shader);
    }
    
    std::cout << "Test completed." << std::endl;
    return 0;
}
