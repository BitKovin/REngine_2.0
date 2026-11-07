$input v_texcoord0

#include <../bgfx_shader.sh>
// Half-size copy of a layer region: four bilinear taps one source texel from the pixel centre (a 4 x 4 tent).
SAMPLER2D(s_fxSrc, 0);
uniform vec4 u_fxClamp;   // min u, min v, max u, max v of the source region, pulled in by half a texel
uniform vec4 u_fxStep;    // xy = uv size of one source texel

vec4 fxTap(vec2 uv)
{
    return texture2DLod(s_fxSrc, clamp(uv, u_fxClamp.xy, u_fxClamp.zw), 0.0);
}

void main()
{
    vec2 uv = v_texcoord0;
    vec2 h = u_fxStep.xy;
    vec4 c = fxTap(uv + vec2(-h.x, -h.y)) + fxTap(uv + vec2(h.x, -h.y)) + fxTap(uv + vec2(-h.x, h.y)) + fxTap(uv + vec2(h.x, h.y));
    gl_FragColor = c * 0.25;
}
