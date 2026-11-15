$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_goreBind

#include <../bgfx_shader.sh>

// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
SAMPLER2D(u_goreDecal,       8);
#include "gore_common.sh"

void main()
{
    if (u_goreLayer.z < 0.5 && v_goreBind.w < 0.004) discard;

    float rim = 0.0; float soak = 0.0; float clot = 0.0; float fresh = 0.0;
    bool anyBits = (u_goreRangeMask.x + u_goreRangeMask.y) > 0.0;
    if (u_goreLayer.w > 1.5) { if (goreCavityRemoved(v_goreBind.xyz, v_color0.r)) discard; }   // a wound cavity
    else if (anyBits && goreSurface(v_goreBind.xyz, rim, soak, clot, fresh)) discard;

    #if(BGFX_SHADER_LANGUAGE_ESSL)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    #endif
}
