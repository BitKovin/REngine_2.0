$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent, v_goreBind

#include <../bgfx_shader.sh>
#include "gore_common.sh"

void main()
{
    vec3 P; float k; float pr; float th;
    if (!goreInterior(v_world.xyz, v_goreBind.xyz, P, k, pr, th)) discard;
    gl_FragDepth = goreDepth(P);
}
