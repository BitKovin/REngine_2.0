#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 FragColor;
uniform sampler2D u_texture;
uniform highp int customId;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1 = (texture (u_texture, v_texcoord) * v_color);
  if ((tmpvar_1.w < 0.999)) {
    discard;
    return;
  };
  highp int tmpvar_2;
  tmpvar_2 = clamp (customId, 0, 16777215);
  vec3 tmpvar_3;
  tmpvar_3.x = (float((
    (tmpvar_2 >> 16)
   & 255)) / 255.0);
  tmpvar_3.y = (float((
    (tmpvar_2 >> 8)
   & 255)) / 255.0);
  tmpvar_3.z = (float((tmpvar_2 & 255)) / 255.0);
  vec4 tmpvar_4;
  tmpvar_4.w = 1.0;
  tmpvar_4.xyz = tmpvar_3;
  FragColor = tmpvar_4;
}

