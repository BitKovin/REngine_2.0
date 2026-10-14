$input v_texcoord0
#include <bgfx_shader.sh>

#if (BGFX_SHADER_LANGUAGE_ESSL && BGFX_SHADER_LANGUAGE_ESSL < 310)
// GLES2/WebGL1 and GLES3.0/WebGL2 — no core sampler2DMS support
void main()
{
    gl_FragDepth = 1.0;
}
#else
// MSAA depth texture (desktop GL, ESSL 310+, HLSL/Metal/SPIRV/PSSL)
SAMPLER2DMS(depthMsaa, 0);
uniform vec4 uResolution;
uniform vec4 uSampleCount;
void main()
{
    vec2 uv = v_texcoord0;
    #if !BGFX_SHADER_LANGUAGE_GLSL
    uv.y = 1.0 - uv.y;
    #endif
    ivec2 texel = ivec2(uv * uResolution.xy);
    float depth = texelFetch(depthMsaa, texel, 0).r;
    int count = int(uSampleCount.x);
    for (int i = 1; i < count; ++i)
    {
        float sampleDepth = texelFetch(depthMsaa, texel, i).r;
        depth = min(depth, sampleDepth);
    }
    gl_FragDepth = depth;
}
#endif