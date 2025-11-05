#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
out lowp vec4 FragColor;
uniform sampler2D u_texture;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1 = (texture (u_texture, v_texcoord) * v_color);
  if ((tmpvar_1.w < 0.01)) {
    discard;
  };
  lowp vec4 tmpvar_2;
  tmpvar_2.xyz = tmpvar_1.xyz;
  tmpvar_2.w = tmpvar_1.w;
  FragColor = tmpvar_2;
}

