#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "glsl_optimizer.h"  // Assuming the header is in include path

namespace fs = std::filesystem;

bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main() {
    fs::path root = fs::current_path();
    fs::path optimized_dir = root / ".optimized";

    // Create the .optimized directory if it doesn't exist
    if (!fs::exists(optimized_dir)) {
        fs::create_directory(optimized_dir);
    }

    // Initialize GLSL optimizer context for GLES 3.0
    glslopt_ctx* ctx = glslopt_initialize(kGlslTargetOpenGLES30);
    if (!ctx) {
        std::cerr << "Failed to initialize GLSL optimizer context." << std::endl;
        return 1;
    }

    // Recursively iterate through all files in the current directory and subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;

        fs::path file_path = entry.path();
        std::string ext = file_path.extension().string();

        // Detect shader type from file extension
        glslopt_shader_type shader_type;
        if (ext == ".vert") {
            shader_type = kGlslOptShaderVertex;
        }
        else if (ext == ".frag") {
            shader_type = kGlslOptShaderFragment;
        }
        else {
            continue;
        }

        // Compute relative path
        fs::path relative_path = fs::relative(file_path, root);

        // Skip files already in .optimized or subfolders
        if (relative_path.string().find(".optimized") == 0) {
            continue;
        }

        // Read shader source
        std::ifstream in(file_path, std::ios::in | std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            continue;
        }
        std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        // Compute output path
        fs::path output_path = optimized_dir / relative_path;

        // Create subdirectories if needed
        fs::create_directories(output_path.parent_path());

        std::string optimized_source;
        if (source.find("//disable_optimization") != std::string::npos) {
            // Skip optimization
            optimized_source = source;
            std::cout << "Skipped optimization for " << relative_path << std::endl;
        }
        else {
            // Optimize the shader
            glslopt_shader* shader = glslopt_optimize(ctx, shader_type, source.c_str(), 0);
            if (!glslopt_get_status(shader)) {
                std::cerr << "Error optimizing " << relative_path << ": " << glslopt_get_log(shader) << std::endl;
                glslopt_shader_delete(shader);
                continue;
            }

            // Get optimized output
            optimized_source = glslopt_get_output(shader);

            // Clean up shader
            glslopt_shader_delete(shader);

            std::cout << "Optimized " << relative_path << std::endl;
        }

        // Write optimized or original shader
        std::ofstream out(output_path);
        if (!out) {
            std::cerr << "Failed to write to: " << output_path << std::endl;
            continue;
        }
        out << optimized_source;
    }

    // Clean up context
    glslopt_cleanup(ctx);

    return 0;
}