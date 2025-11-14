#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_light;
out lowp vec4 FragColor;
uniform sampler2D u_texture;
uniform sampler2D u_textureEmissive;
uniform bool masked;
uniform vec3 light_color;
uniform vec3 direct_light_color;
uniform vec3 direct_light_dir;
void main ()
{
  lowp vec3 color_1;
  lowp vec4 texColor_2;
  if (masked) {
    texColor_2 = (textureLod (u_texture, v_texcoord, 0.0) * v_color);
  } else {
    texColor_2 = (texture (u_texture, v_texcoord) * v_color);
  };
  color_1 = texColor_2.xyz;
  lowp float tmpvar_3;
  tmpvar_3 = texColor_2.w;
  if ((masked && (texColor_2.w < 0.999))) {
    discard;
    return;
  };
  vec3 light_4;
  vec3 normal_5;
  vec3 tmpvar_6;
  tmpvar_6 = normalize(v_normal);
  normal_5 = tmpvar_6;
  if (!(gl_FrontFacing)) {
    normal_5 = -(tmpvar_6);
  };
  light_4 = ((light_color + (
    clamp (((dot (normal_5, 
      normalize(direct_light_dir)
    ) * 0.8) + 0.2), 0.0, 1.0)
   * direct_light_color)) * 2.5);
  light_4 = (light_4 + v_light);
  color_1 = (texColor_2.xyz * (light_4 + texture (u_textureEmissive, v_texcoord).xyz));
  lowp vec4 tmpvar_7;
  tmpvar_7.xyz = color_1;
  tmpvar_7.w = tmpvar_3;
  FragColor = tmpvar_7;
}

