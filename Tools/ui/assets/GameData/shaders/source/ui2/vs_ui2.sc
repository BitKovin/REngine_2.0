$input a_position, a_texcoord0, a_texcoord1, a_texcoord2
$output v_texcoord0, v_local, v_misc

#include <../bgfx_shader.sh>

uniform mat4 u_uiProjection;

void main()
{
    v_texcoord0 = a_texcoord0;
    v_local = a_texcoord1;
    // x = primitive index, yz = position in UI units (for clipping), w unused
    v_misc = vec4(a_texcoord2.x, a_position.x, a_position.y, 0.0);
    gl_Position = mul(u_uiProjection, vec4(a_position.xy, 0.0, 1.0));
}
