#version 300 es
layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 TextureCoordinate;
layout(location=5) in highp ivec4 boneIds;
layout(location=6) in vec4 weights;
layout(location=8) in vec4 color;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform mat4 lightMatrix1;
uniform mat4 lightMatrix2;
uniform mat4 lightMatrix3;
uniform mat4 lightMatrix4;
uniform float brightness;
uniform float viewmodelScaleFactor;
uniform mat4 finalBonesMatrices[128];
uniform bool isViewmodel;
out vec2 v_texcoord;
out vec4 v_color;
out vec3 v_normal;
out vec3 v_worldPosition;
out vec4 v_clipPosition;
out vec4 v_shadowCoords1;
out vec4 v_shadowCoords2;
out vec4 v_shadowCoords3;
out vec4 v_shadowCoords4;
out vec3 v_light;
uniform highp int PointLightsNumber;
uniform vec4 LightPositions[16];
uniform vec3 LightColors[16];
uniform float LightRadiuses[16];
uniform vec4 LightDirections[16];
void main ()
{
  highp int i_1;
  mat4 tmpvar_2;
  float tmpvar_3;
  tmpvar_3 = ((weights.x + weights.y) + (weights.z + weights.w));
  if ((tmpvar_3 < 0.01)) {
    tmpvar_2 = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
  } else {
    tmpvar_2 = (((
      ((finalBonesMatrices[boneIds.x] * weights.x) / tmpvar_3)
     + 
      ((finalBonesMatrices[boneIds.y] * weights.y) / tmpvar_3)
    ) + (
      (finalBonesMatrices[boneIds.z] * weights.z)
     / tmpvar_3)) + ((finalBonesMatrices[boneIds.w] * weights.w) / tmpvar_3));
  };
  mat4 tmpvar_4;
  tmpvar_4 = (world * tmpvar_2);
  vec4 tmpvar_5;
  tmpvar_5.w = 1.0;
  tmpvar_5.xyz = Position;
  v_clipPosition = (((projection * view) * tmpvar_4) * tmpvar_5);
  if (isViewmodel) {
    v_clipPosition.z = (v_clipPosition.z * (0.02 * viewmodelScaleFactor));
  };
  gl_Position = v_clipPosition;
  vec4 tmpvar_6;
  tmpvar_6.w = 1.0;
  tmpvar_6.xyz = Position;
  v_worldPosition = (tmpvar_4 * tmpvar_6).xyz;
  vec3 tmpvar_7;
  vec3 tmpvar_8;
  vec3 tmpvar_9;
  tmpvar_7 = tmpvar_4[uint(0)].xyz;
  tmpvar_8 = tmpvar_4[1u].xyz;
  tmpvar_9 = tmpvar_4[2u].xyz;
  mat3 tmpvar_10;
  float tmpvar_11;
  float tmpvar_12;
  float tmpvar_13;
  tmpvar_11 = ((tmpvar_8.y * tmpvar_9.z) - (tmpvar_9.y * tmpvar_8.z));
  tmpvar_12 = ((tmpvar_8.x * tmpvar_9.z) - (tmpvar_9.x * tmpvar_8.z));
  tmpvar_13 = ((tmpvar_8.x * tmpvar_9.y) - (tmpvar_9.x * tmpvar_8.y));
  mat3 tmpvar_14;
  tmpvar_14[0].x = tmpvar_11;
  tmpvar_14[1].x = -(tmpvar_12);
  tmpvar_14[2].x = tmpvar_13;
  tmpvar_14[0].y = ((tmpvar_9.y * tmpvar_7.z) - (tmpvar_7.y * tmpvar_9.z));
  tmpvar_14[1].y = ((tmpvar_7.x * tmpvar_9.z) - (tmpvar_9.x * tmpvar_7.z));
  tmpvar_14[2].y = ((tmpvar_9.x * tmpvar_7.y) - (tmpvar_7.x * tmpvar_9.y));
  tmpvar_14[0].z = ((tmpvar_7.y * tmpvar_8.z) - (tmpvar_8.y * tmpvar_7.z));
  tmpvar_14[1].z = ((tmpvar_8.x * tmpvar_7.z) - (tmpvar_7.x * tmpvar_8.z));
  tmpvar_14[2].z = ((tmpvar_7.x * tmpvar_8.y) - (tmpvar_8.x * tmpvar_7.y));
  tmpvar_10 = (tmpvar_14 / ((
    (tmpvar_7.x * tmpvar_11)
   - 
    (tmpvar_7.y * tmpvar_12)
  ) + (tmpvar_7.z * tmpvar_13)));
  mat3 tmpvar_15;
  tmpvar_15[0].x = tmpvar_10[0].x;
  tmpvar_15[1].x = tmpvar_10[0].y;
  tmpvar_15[2].x = tmpvar_10[0].z;
  tmpvar_15[0].y = tmpvar_10[1].x;
  tmpvar_15[1].y = tmpvar_10[1].y;
  tmpvar_15[2].y = tmpvar_10[1].z;
  tmpvar_15[0].z = tmpvar_10[2].x;
  tmpvar_15[1].z = tmpvar_10[2].y;
  tmpvar_15[2].z = tmpvar_10[2].z;
  v_normal = normalize((tmpvar_15 * Normal));
  vec4 tmpvar_16;
  tmpvar_16.w = 1.0;
  tmpvar_16.x = brightness;
  tmpvar_16.y = brightness;
  tmpvar_16.z = brightness;
  v_color = (tmpvar_16 * color);
  v_texcoord = TextureCoordinate;
  vec4 tmpvar_17;
  tmpvar_17.w = 1.0;
  tmpvar_17.xyz = Position;
  v_shadowCoords1 = ((lightMatrix1 * tmpvar_4) * tmpvar_17);
  vec4 tmpvar_18;
  tmpvar_18.w = 1.0;
  tmpvar_18.xyz = Position;
  v_shadowCoords2 = ((lightMatrix2 * tmpvar_4) * tmpvar_18);
  vec4 tmpvar_19;
  tmpvar_19.w = 1.0;
  tmpvar_19.xyz = Position;
  v_shadowCoords3 = ((lightMatrix3 * tmpvar_4) * tmpvar_19);
  vec4 tmpvar_20;
  tmpvar_20.w = 1.0;
  tmpvar_20.xyz = Position;
  v_shadowCoords4 = ((lightMatrix4 * tmpvar_4) * tmpvar_20);
  v_light = vec3(0.0, 0.0, 0.0);
  i_1 = 0;
  while (true) {
    highp int tmpvar_21;
    tmpvar_21 = min (16, PointLightsNumber);
    if ((i_1 >= tmpvar_21)) {
      break;
    };
    vec3 tmpvar_22;
    float intense_23;
    vec3 tmpvar_24;
    tmpvar_24 = normalize(v_normal);
    vec3 tmpvar_25;
    tmpvar_25 = (LightPositions[i_1].xyz - v_worldPosition);
    float tmpvar_26;
    tmpvar_26 = sqrt(dot (tmpvar_25, tmpvar_25));
    if (((tmpvar_26 > LightRadiuses[i_1]) || (LightRadiuses[i_1] <= 0.0))) {
      tmpvar_22 = vec3(0.0, 0.0, 0.0);
    } else {
      vec3 tmpvar_27;
      tmpvar_27 = (tmpvar_25 / tmpvar_26);
      float tmpvar_28;
      tmpvar_28 = LightDirections[i_1].w;
      float tmpvar_29;
      float tmpvar_30;
      tmpvar_30 = clamp (((
        dot (-(tmpvar_27), normalize(LightDirections[i_1].xyz))
       - tmpvar_28) / (LightPositions[i_1].w - tmpvar_28)), 0.0, 1.0);
      tmpvar_29 = (tmpvar_30 * (tmpvar_30 * (3.0 - 
        (2.0 * tmpvar_30)
      )));
      if ((tmpvar_29 <= 0.001)) {
        tmpvar_22 = vec3(0.0, 0.0, 0.0);
      } else {
        float tmpvar_31;
        tmpvar_31 = dot (tmpvar_24, tmpvar_27);
        if ((tmpvar_31 < 0.0)) {
          tmpvar_22 = vec3(0.0, 0.0, 0.0);
        } else {
          intense_23 = (max ((
            (LightRadiuses[i_1] - tmpvar_26)
           / LightRadiuses[i_1]), 0.0) * max (dot (tmpvar_24, tmpvar_27), 0.0));
          float tmpvar_32;
          tmpvar_32 = max (intense_23, 0.0);
          intense_23 = tmpvar_32;
          tmpvar_22 = ((LightColors[i_1] * tmpvar_32) * tmpvar_29);
        };
      };
    };
    v_light = (v_light + tmpvar_22);
    i_1++;
  };
}

