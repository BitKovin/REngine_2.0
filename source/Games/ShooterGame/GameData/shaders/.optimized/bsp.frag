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
  lowp vec4 o_texture_4;
  o_texture_4 = texture (s_bspTexture, g_TexCoord);
  vec3 tmpvar_5;
  tmpvar_5 = normalize(g_normal);
  normal_2 = tmpvar_5;
  o_lightmap_3 = (((texture (s_bspLightmap, g_LmapCoord).xyz * light_color) * (4.04 * g_color.xyz)) + ((
    clamp (((dot (tmpvar_5, 
      normalize(direct_light_dir)
    ) * 0.7) + 0.3), 0.0, 1.0)
   * direct_light_color) * 2.0));
  i_1 = 0;
  while (true) {
    highp int tmpvar_6;
    tmpvar_6 = min (16, PointLightsNumber);
    if ((i_1 >= tmpvar_6)) {
      break;
    };
    vec3 tmpvar_7;
    float intense_8;
    vec3 tmpvar_9;
    tmpvar_9 = normalize(normal_2);
    vec3 tmpvar_10;
    tmpvar_10 = (LightPositions[i_1].xyz - g_world.xyz);
    float tmpvar_11;
    tmpvar_11 = sqrt(dot (tmpvar_10, tmpvar_10));
    if (((tmpvar_11 > LightRadiuses[i_1]) || (LightRadiuses[i_1] <= 0.0))) {
      tmpvar_7 = vec3(0.0, 0.0, 0.0);
    } else {
      vec3 tmpvar_12;
      tmpvar_12 = (tmpvar_10 / tmpvar_11);
      float tmpvar_13;
      tmpvar_13 = LightDirections[i_1].w;
      float tmpvar_14;
      float tmpvar_15;
      tmpvar_15 = clamp (((
        dot (-(tmpvar_12), normalize(LightDirections[i_1].xyz))
       - tmpvar_13) / (LightPositions[i_1].w - tmpvar_13)), 0.0, 1.0);
      tmpvar_14 = (tmpvar_15 * (tmpvar_15 * (3.0 - 
        (2.0 * tmpvar_15)
      )));
      if ((tmpvar_14 <= 0.001)) {
        tmpvar_7 = vec3(0.0, 0.0, 0.0);
      } else {
        float tmpvar_16;
        tmpvar_16 = dot (tmpvar_9, tmpvar_12);
        if ((tmpvar_16 < 0.0)) {
          tmpvar_7 = vec3(0.0, 0.0, 0.0);
        } else {
          intense_8 = (max ((
            (LightRadiuses[i_1] - tmpvar_11)
           / LightRadiuses[i_1]), 0.0) * max (dot (tmpvar_9, tmpvar_12), 0.0));
          float tmpvar_17;
          tmpvar_17 = max (intense_8, 0.0);
          intense_8 = tmpvar_17;
          tmpvar_7 = ((LightColors[i_1] * tmpvar_17) * tmpvar_14);
        };
      };
    };
    o_lightmap_3 = (o_lightmap_3 + tmpvar_7);
    i_1++;
  };
  lowp vec4 tmpvar_18;
  tmpvar_18.w = 1.0;
  tmpvar_18.xyz = o_lightmap_3;
  lowp vec4 fragColor_19;
  fragColor_19 = (o_texture_4 * tmpvar_18);
  vec3 tmpvar_20;
  tmpvar_20 = (g_world.xyz - cameraPosition);
  lowp vec4 tmpvar_21;
  tmpvar_21.xyz = mix (fragColor_19.xyz, fog_color, (clamp (
    ((sqrt(dot (tmpvar_20, tmpvar_20)) - fog_start) / (fog_end - fog_start))
  , 0.0, 1.0) * fog_opacity));
  tmpvar_21.w = fragColor_19.w;
  FragColor = tmpvar_21;
}

