#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
uniform sampler2D u_texture;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1 = (textureLod (u_texture, v_texcoord, 0.0) * v_color);
  if ((tmpvar_1.w < 0.99)) {
    discard;
    return;
  };
}

