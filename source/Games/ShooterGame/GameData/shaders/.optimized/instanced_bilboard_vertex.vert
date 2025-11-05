#version 300 es
layout(location=0) in vec3 Position;
layout(location=2) in vec2 TextureCoordinate;
layout(location=10) in vec4 instanceModel0;
layout(location=11) in vec4 instanceModel1;
layout(location=12) in vec4 instanceModel2;
layout(location=13) in vec4 instanceModel3;
layout(location=14) in vec4 instanceColor;
uniform float brightness;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightMatrix1;
uniform mat4 lightMatrix2;
uniform mat4 lightMatrix3;
uniform mat4 lightMatrix4;
out vec2 v_texcoord;
out vec4 v_color;
out vec3 v_normal;
out vec3 v_worldPosition;
out highp vec4 v_clipPosition;
out vec4 v_shadowCoords1;
out vec4 v_shadowCoords2;
out vec4 v_shadowCoords3;
out vec4 v_shadowCoords4;
uniform bool is_decal;
void main ()
{
  mat4 tmpvar_1;
  tmpvar_1[uint(0)] = instanceModel0;
  tmpvar_1[1u] = instanceModel1;
  tmpvar_1[2u] = instanceModel2;
  tmpvar_1[3u] = instanceModel3;
  vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.xyz = Position;
  vec4 tmpvar_3;
  tmpvar_3 = (tmpvar_1 * tmpvar_2);
  v_worldPosition = tmpvar_3.xyz;
  gl_Position = ((projection * view) * tmpvar_3);
  v_clipPosition = gl_Position;
  v_texcoord = TextureCoordinate;
  vec4 tmpvar_4;
  tmpvar_4.w = 1.0;
  tmpvar_4.x = brightness;
  tmpvar_4.y = brightness;
  tmpvar_4.z = brightness;
  v_color = (instanceColor * tmpvar_4);
  v_normal = vec3(0.0, 1.0, 0.0);
  if (is_decal) {
    v_normal = normalize(tmpvar_1[2].xyz);
  };
  v_shadowCoords1 = (lightMatrix1 * tmpvar_3);
  v_shadowCoords2 = (lightMatrix2 * tmpvar_3);
  v_shadowCoords3 = (lightMatrix3 * tmpvar_3);
  v_shadowCoords4 = (lightMatrix4 * tmpvar_3);
}

