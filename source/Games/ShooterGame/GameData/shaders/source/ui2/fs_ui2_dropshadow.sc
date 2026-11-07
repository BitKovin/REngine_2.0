$input v_texcoord0

#include <../bgfx_shader.sh>
// Draws a layer region into another one. Mode 0: copy, scaled by the opacity in u_fxColor.x.
// Mode 1: the shadow of the region, its alpha tinted with the colour u_fxColor (straight RGBA); the quad's position supplies the offset.
SAMPLER2D(s_fxSrc, 0);
uniform vec4 u_fxClamp;   // min u, min v, max u, max v of the source region, pulled in by half a texel
uniform vec4 u_fxColor;
uniform vec4 u_fxMode;    // x: 0 copy, 1 shadow

void main()
{
    vec4 t = texture2DLod(s_fxSrc, clamp(v_texcoord0, u_fxClamp.xy, u_fxClamp.zw), 0.0);
    if (u_fxMode.x < 0.5)
        gl_FragColor = t * u_fxColor.x;
    else
        gl_FragColor = vec4(u_fxColor.rgb * (u_fxColor.a * t.a), u_fxColor.a * t.a);
}
