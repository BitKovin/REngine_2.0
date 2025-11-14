#version 300 es
precision highp float;
in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_normal;
in vec4 g_color;
in vec4 g_world;
uniform vec3 cameraPosition;
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;
uniform vec3 light_color;
uniform vec3 direct_light_color;
uniform vec3 direct_light_dir;
uniform float fog_start;
uniform float fog_end;
uniform float fog_opacity;
uniform vec3 fog_color;
uniform bool useVertexLight;
out lowp vec4 FragColor;
uniform highp int PointLightsNumber;
uniform vec4 LightPositions[16];
uniform vec3 LightColors[16];
uniform float LightRadiuses[16];
uniform vec4 LightDirections[16];
void main ()
{
  highp int i_1;
  vec3 normal_2;
  lowp vec3 o_lightmap_3;
  vec3 vertexLightFactor_4;
  lowp vec4 o_texture_5;
  o_texture_5 = texture (s_bspTexture, g_TexCoord);
  vertexLightFactor_4 = vec3(1.5, 1.5, 1.5);
  if (useVertexLight) {
    vertexLightFactor_4 = (4.04 * g_color.xyz);
  };
  vertexLightFactor_4 = (vertexLightFactor_4 * 1.5);
  vec3 tmpvar_6;
  tmpvar_6 = normalize(g_normal);
  normal_2 = tmpvar_6;
  o_lightmap_3 = (((texture (s_bspLightmap, g_LmapCoord).xyz * light_color) * vertexLightFactor_4) + ((
    clamp (((dot (tmpvar_6, 
      normalize(direct_light_dir)
    ) * 0.7) + 0.3), 0.0, 1.0)
   * direct_light_color) * 2.0));
  i_1 = 0;
  while (true) {
    highp int tmpvar_7;
    tmpvar_7 = min (16, PointLightsNumber);
    if ((i_1 >= tmpvar_7)) {
      break;
    };
    vec3 tmpvar_8;
    float intense_9;
    vec3 tmpvar_10;
    tmpvar_10 = normalize(normal_2);
    vec3 tmpvar_11;
    tmpvar_11 = (LightPositions[i_1].xyz - g_world.xyz);
    float tmpvar_12;
    tmpvar_12 = sqrt(dot (tmpvar_11, tmpvar_11));
    if (((tmpvar_12 > LightRadiuses[i_1]) || (LightRadiuses[i_1] <= 0.0))) {
      tmpvar_8 = vec3(0.0, 0.0, 0.0);
    } else {
      vec3 tmpvar_13;
      tmpvar_13 = (tmpvar_11 / tmpvar_12);
      float tmpvar_14;
      tmpvar_14 = LightDirections[i_1].w;
      float tmpvar_15;
      float tmpvar_16;
      tmpvar_16 = clamp (((
        dot (-(tmpvar_13), normalize(LightDirections[i_1].xyz))
       - tmpvar_14) / (LightPositions[i_1].w - tmpvar_14)), 0.0, 1.0);
      tmpvar_15 = (tmpvar_16 * (tmpvar_16 * (3.0 - 
        (2.0 * tmpvar_16)
      )));
      if ((tmpvar_15 <= 0.001)) {
        tmpvar_8 = vec3(0.0, 0.0, 0.0);
      } else {
        float tmpvar_17;
        tmpvar_17 = dot (tmpvar_10, tmpvar_13);
        if ((tmpvar_17 < 0.0)) {
          tmpvar_8 = vec3(0.0, 0.0, 0.0);
        } else {
          intense_9 = (max ((
            (LightRadiuses[i_1] - tmpvar_12)
           / LightRadiuses[i_1]), 0.0) * max (dot (tmpvar_10, tmpvar_13), 0.0));
          float tmpvar_18;
          tmpvar_18 = max (intense_9, 0.0);
          intense_9 = tmpvar_18;
          tmpvar_8 = ((LightColors[i_1] * tmpvar_18) * tmpvar_15);
        };
      };
    };
    o_lightmap_3 = (o_lightmap_3 + tmpvar_8);
    i_1++;
  };
  lowp vec4 tmpvar_19;
  tmpvar_19.w = 1.0;
  tmpvar_19.xyz = o_lightmap_3;
  lowp vec4 fragColor_20;
  fragColor_20 = (o_texture_5 * tmpvar_19);
  vec3 tmpvar_21;
  tmpvar_21 = (g_world.xyz - cameraPosition);
  lowp vec4 tmpvar_22;
  tmpvar_22.xyz = mix (fragColor_20.xyz, fog_color, (clamp (
    ((sqrt(dot (tmpvar_21, tmpvar_21)) - fog_start) / (fog_end - fog_start))
  , 0.0, 1.0) * fog_opacity));
  tmpvar_22.w = fragColor_20.w;
  FragColor = tmpvar_22;
}

