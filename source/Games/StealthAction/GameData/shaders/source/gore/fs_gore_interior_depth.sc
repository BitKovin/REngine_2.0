$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind

#include <../bgfx_shader.sh>
// Sampler slots are parsed from this file's own text by the engine (includes are not scanned), so they are declared here.
SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);
#include "gore_common.sh"

void main()
{
    vec3 P; float k; float pr; float th;
    if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, k, pr, th)) discard;
    gl_FragDepth = goreDepth(P);
}
