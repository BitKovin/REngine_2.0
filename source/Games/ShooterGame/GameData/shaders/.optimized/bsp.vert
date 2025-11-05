#version 300 es
layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 TextureCoordinate;
layout(location=8) in vec4 Color;
layout(location=9) in vec2 ShadowMapCoords;
out vec2 g_TexCoord;
out vec2 g_LmapCoord;
out vec3 g_normal;
out vec4 g_color;
out vec4 g_world;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = Position;
  g_world = (model * tmpvar_1);
  gl_Position = ((projection * view) * g_world);
  vec3 tmpvar_2;
  vec3 tmpvar_3;
  vec3 tmpvar_4;
  tmpvar_2 = model[uint(0)].xyz;
  tmpvar_3 = model[1u].xyz;
  tmpvar_4 = model[2u].xyz;
  mat3 tmpvar_5;
  float tmpvar_6;
  float tmpvar_7;
  float tmpvar_8;
  tmpvar_6 = ((tmpvar_3.y * tmpvar_4.z) - (tmpvar_4.y * tmpvar_3.z));
  tmpvar_7 = ((tmpvar_3.x * tmpvar_4.z) - (tmpvar_4.x * tmpvar_3.z));
  tmpvar_8 = ((tmpvar_3.x * tmpvar_4.y) - (tmpvar_4.x * tmpvar_3.y));
  mat3 tmpvar_9;
  tmpvar_9[0].x = tmpvar_6;
  tmpvar_9[1].x = -(tmpvar_7);
  tmpvar_9[2].x = tmpvar_8;
  tmpvar_9[0].y = ((tmpvar_4.y * tmpvar_2.z) - (tmpvar_2.y * tmpvar_4.z));
  tmpvar_9[1].y = ((tmpvar_2.x * tmpvar_4.z) - (tmpvar_4.x * tmpvar_2.z));
  tmpvar_9[2].y = ((tmpvar_4.x * tmpvar_2.y) - (tmpvar_2.x * tmpvar_4.y));
  tmpvar_9[0].z = ((tmpvar_2.y * tmpvar_3.z) - (tmpvar_3.y * tmpvar_2.z));
  tmpvar_9[1].z = ((tmpvar_3.x * tmpvar_2.z) - (tmpvar_2.x * tmpvar_3.z));
  tmpvar_9[2].z = ((tmpvar_2.x * tmpvar_3.y) - (tmpvar_3.x * tmpvar_2.y));
  tmpvar_5 = (tmpvar_9 / ((
    (tmpvar_2.x * tmpvar_6)
   - 
    (tmpvar_2.y * tmpvar_7)
  ) + (tmpvar_2.z * tmpvar_8)));
  mat3 tmpvar_10;
  tmpvar_10[0].x = tmpvar_5[0].x;
  tmpvar_10[1].x = tmpvar_5[0].y;
  tmpvar_10[2].x = tmpvar_5[0].z;
  tmpvar_10[0].y = tmpvar_5[1].x;
  tmpvar_10[1].y = tmpvar_5[1].y;
  tmpvar_10[2].y = tmpvar_5[1].z;
  tmpvar_10[0].z = tmpvar_5[2].x;
  tmpvar_10[1].z = tmpvar_5[2].y;
  tmpvar_10[2].z = tmpvar_5[2].z;
  g_normal = normalize((tmpvar_10 * Normal));
  g_TexCoord = TextureCoordinate;
  g_LmapCoord = ShadowMapCoords;
  g_color = Color;
}

