$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind

#include <../bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

uniform vec4 u_goreAlphaTest;   // x > 0.5: alpha-tested like fs_mask (set by GoreDrawCommand)

#include "gore_common.sh"

void main()
{
    float rim = 0.0; float soak = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y + u_goreRangeMask.z) > 0.0;
    if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, fresh)) discard;

    if (u_goreAlphaTest.x > 0.5)
    {
        vec4 texColor = texture2D(u_texture, v_texcoord0) * v_color0;
        if (texColor.a < 0.99)
            discard;
    }

    #if(BGFX_SHADER_LANGUAGE_ESSL)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    #endif
}
