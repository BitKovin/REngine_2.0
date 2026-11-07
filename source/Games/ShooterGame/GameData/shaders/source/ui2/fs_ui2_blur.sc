$input v_texcoord0

#include <../bgfx_shader.sh>
// One direction of a separable Gaussian over a layer region: 9 taps (1 centre + 4 linear-sampled pairs), weights from the CPU.
SAMPLER2D(s_fxSrc, 0);
uniform vec4 u_fxClamp;   // min u, min v, max u, max v of the source region, pulled in by half a texel
uniform vec4 u_fxStep;    // xy = uv distance of one source texel along the blur axis (the other component is 0)
uniform vec4 u_fxW0;      // centre weight, pair weights 1..3
uniform vec4 u_fxW1;      // pair weight 4
uniform vec4 u_fxO0;      // pair offsets 1..4 in texels

vec4 fxTap(vec2 uv)
{
    return texture2DLod(s_fxSrc, clamp(uv, u_fxClamp.xy, u_fxClamp.zw), 0.0);
}

void main()
{
    vec2 uv = v_texcoord0;
    vec2 s = u_fxStep.xy;
    vec4 c = fxTap(uv) * u_fxW0.x;
    c += (fxTap(uv + s * u_fxO0.x) + fxTap(uv - s * u_fxO0.x)) * u_fxW0.y;
    c += (fxTap(uv + s * u_fxO0.y) + fxTap(uv - s * u_fxO0.y)) * u_fxW0.z;
    c += (fxTap(uv + s * u_fxO0.z) + fxTap(uv - s * u_fxO0.z)) * u_fxW0.w;
    c += (fxTap(uv + s * u_fxO0.w) + fxTap(uv - s * u_fxO0.w)) * u_fxW1.x;
    gl_FragColor = c;
}
