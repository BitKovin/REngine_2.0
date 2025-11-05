#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec4 v_clipPosition;
in vec4 v_shadowCoords1;
in vec4 v_shadowCoords2;
in vec4 v_shadowCoords3;
in vec4 v_shadowCoords4;
out lowp vec4 FragColor;
uniform sampler2D u_texture;
uniform highp sampler2DShadow shadowMap;
uniform highp sampler2DShadow shadowMapDetail;
uniform highp int shadowMapSize;
uniform vec3 lightDirection;
uniform float shadowDistance1;
uniform float shadowDistance2;
uniform float shadowDistance3;
uniform float shadowDistance4;
uniform float shadowRadius1;
uniform float shadowRadius2;
uniform float shadowRadius3;
uniform float shadowRadius4;
uniform bool is_particle;
void main ()
{
  lowp vec3 color_1;
  lowp vec4 tmpvar_2;
  tmpvar_2 = (texture (u_texture, v_texcoord) * v_color);
  color_1 = tmpvar_2.xyz;
  lowp float tmpvar_3;
  tmpvar_3 = tmpvar_2.w;
  if ((tmpvar_2.w < 0.01)) {
    discard;
  };
  highp vec2 shadow_4;
  vec4 cascadeCoords_5[4];
  float cascadeRadius_6[4];
  float cascadeDist_7[4];
  float dist_8;
  highp float indirectPower_9;
  highp float directionPower_10;
  highp float tmpvar_11;
  if (is_particle) {
    tmpvar_11 = 1.0;
  } else {
    tmpvar_11 = clamp (dot (-(v_normal), lightDirection), 0.0, 1.0);
  };
  directionPower_10 = tmpvar_11;
  indirectPower_9 = ((dot (
    -(v_normal)
  , vec3(0.0, -1.0, 0.0)) / 2.0) + 0.5);
  if (is_particle) {
    indirectPower_9 = 1.0;
  };
  highp float tmpvar_12;
  tmpvar_12 = mix (0.2, 0.35, indirectPower_9);
  indirectPower_9 = tmpvar_12;
  dist_8 = v_clipPosition.z;
  float tmpvar_13[4];
  tmpvar_13[0] = shadowDistance1;
  tmpvar_13[1] = shadowDistance2;
  tmpvar_13[2] = shadowDistance3;
  tmpvar_13[3] = shadowDistance4;
  cascadeDist_7 = tmpvar_13;
  float tmpvar_14[4];
  tmpvar_14[0] = shadowRadius1;
  tmpvar_14[1] = shadowRadius2;
  tmpvar_14[2] = shadowRadius3;
  tmpvar_14[3] = shadowRadius4;
  cascadeRadius_6 = tmpvar_14;
  vec4 tmpvar_15[4];
  tmpvar_15[0] = v_shadowCoords1;
  tmpvar_15[1] = v_shadowCoords2;
  tmpvar_15[2] = v_shadowCoords3;
  tmpvar_15[3] = v_shadowCoords4;
  cascadeCoords_5 = tmpvar_15;
  shadow_4 = vec2(1.0, 1.0);
  if ((v_clipPosition.z < tmpvar_13[3])) {
    bool blended_17;
    blended_17 = bool(0);
    for (highp int i_16 = 0; i_16 < 3; i_16++) {
      float tmpvar_18;
      if ((i_16 == 0)) {
        tmpvar_18 = 0.0;
      } else {
        tmpvar_18 = cascadeDist_7[(i_16 - 1)];
      };
      float tmpvar_19;
      tmpvar_19 = (cascadeDist_7[i_16] - ((cascadeDist_7[i_16] - tmpvar_18) * 0.15));
      float tmpvar_20;
      tmpvar_20 = cascadeDist_7[i_16];
      if (((dist_8 >= tmpvar_19) && (dist_8 <= tmpvar_20))) {
        float tmpvar_21;
        float tmpvar_22;
        tmpvar_22 = clamp (((dist_8 - tmpvar_19) / (tmpvar_20 - tmpvar_19)), 0.0, 1.0);
        tmpvar_21 = (tmpvar_22 * (tmpvar_22 * (3.0 - 
          (2.0 * tmpvar_22)
        )));
        float shadowDistance_23;
        shadowDistance_23 = cascadeRadius_6[i_16];
        vec4 shadowCoords_24;
        shadowCoords_24 = cascadeCoords_5[i_16];
        highp int pcfRadius_25;
        pcfRadius_25 = int[4](2, 1, 1, 1)[i_16];
        highp int xo_26;
        highp float sampleScale_27;
        highp float sumIndirect_28;
        highp int xo_29;
        float n_30;
        highp float sumDirect_31;
        vec2 texelSize_32;
        float bias_33;
        vec3 pc_34;
        pc_34 = (((
          (shadowCoords_24 / shadowCoords_24.w)
        .xyz * 0.5) + 0.5) * vec3(0.5, 0.5, 1.0));
        vec3 tmpvar_35;
        tmpvar_35.z = 0.0;
        tmpvar_35.xy = (vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[i_16] * vec2(0.5, 0.5));
        pc_34 = (pc_34 + tmpvar_35);
        float tmpvar_36;
        tmpvar_36 = float(shadowMapSize);
        bias_33 = (((
          (float[4](1.0, 1.0, 1.0, 1.0)[i_16] * ((2.0 * shadowDistance_23) / tmpvar_36))
         * 
          (1.0 + (1.0 - clamp (dot (
            normalize(v_normal)
          , 
            normalize(lightDirection)
          ), 0.0, 1.0)))
        ) / (shadowDistance_23 + 1000.0)) * (float(pcfRadius_25) + 1.0));
        texelSize_32 = (0.5 / vec2(tmpvar_36));
        sumDirect_31 = 0.0;
        n_30 = 0.0;
        xo_29 = -(pcfRadius_25);
        while (true) {
          highp int yo_37;
          if ((xo_29 > pcfRadius_25)) {
            break;
          };
          yo_37 = -(pcfRadius_25);
          while (true) {
            if ((yo_37 > pcfRadius_25)) {
              break;
            };
            n_30 += 1.0;
            vec2 tmpvar_38;
            tmpvar_38.x = float(xo_29);
            tmpvar_38.y = float(yo_37);
            vec3 tmpvar_39;
            tmpvar_39.xy = (pc_34.xy + (tmpvar_38 * texelSize_32));
            tmpvar_39.z = (pc_34.z - bias_33);
            sumDirect_31 = (sumDirect_31 + texture (shadowMap, tmpvar_39));
            yo_37++;
          };
          xo_29++;
        };
        highp float tmpvar_40;
        tmpvar_40 = (sumDirect_31 / n_30);
        sumIndirect_28 = 0.0;
        sampleScale_27 = mix (4.0, 1.0, tmpvar_40);
        n_30 = 0.0;
        xo_26 = -(pcfRadius_25);
        while (true) {
          highp int yo_41;
          if ((xo_26 > pcfRadius_25)) {
            break;
          };
          yo_41 = -(pcfRadius_25);
          while (true) {
            if ((yo_41 > pcfRadius_25)) {
              break;
            };
            n_30 += 1.0;
            vec2 tmpvar_42;
            tmpvar_42.x = float(xo_26);
            tmpvar_42.y = float(yo_41);
            highp vec3 tmpvar_43;
            tmpvar_43.xy = (pc_34.xy + ((tmpvar_42 * texelSize_32) * sampleScale_27));
            tmpvar_43.z = (pc_34.z - (bias_33 * sampleScale_27));
            sumIndirect_28 = (sumIndirect_28 + texture (shadowMapDetail, tmpvar_43));
            yo_41++;
          };
          xo_26++;
        };
        highp float tmpvar_44;
        tmpvar_44 = (sumIndirect_28 / n_30);
        highp vec2 tmpvar_45;
        tmpvar_45.x = (tmpvar_40 * tmpvar_44);
        tmpvar_45.y = mix (((tmpvar_44 / 2.0) + 0.5), 1.0, tmpvar_40);
        float shadowDistance_46;
        shadowDistance_46 = cascadeRadius_6[(i_16 + 1)];
        vec4 shadowCoords_47;
        shadowCoords_47 = cascadeCoords_5[(i_16 + 1)];
        highp int pcfRadius_48;
        pcfRadius_48 = int[4](2, 1, 1, 1)[(i_16 + 1)];
        highp int xo_49;
        highp float sampleScale_50;
        highp float sumIndirect_51;
        highp int xo_52;
        float n_53;
        highp float sumDirect_54;
        vec2 texelSize_55;
        float bias_56;
        vec3 pc_57;
        pc_57 = (((
          (shadowCoords_47 / shadowCoords_47.w)
        .xyz * 0.5) + 0.5) * vec3(0.5, 0.5, 1.0));
        vec3 tmpvar_58;
        tmpvar_58.z = 0.0;
        tmpvar_58.xy = (vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[(i_16 + 1)] * vec2(0.5, 0.5));
        pc_57 = (pc_57 + tmpvar_58);
        float tmpvar_59;
        tmpvar_59 = float(shadowMapSize);
        bias_56 = (((
          (float[4](1.0, 1.0, 1.0, 1.0)[(i_16 + 1)] * ((2.0 * shadowDistance_46) / tmpvar_59))
         * 
          (1.0 + (1.0 - clamp (dot (
            normalize(v_normal)
          , 
            normalize(lightDirection)
          ), 0.0, 1.0)))
        ) / (shadowDistance_46 + 1000.0)) * (float(pcfRadius_48) + 1.0));
        texelSize_55 = (0.5 / vec2(tmpvar_59));
        sumDirect_54 = 0.0;
        n_53 = 0.0;
        xo_52 = -(pcfRadius_48);
        while (true) {
          highp int yo_60;
          if ((xo_52 > pcfRadius_48)) {
            break;
          };
          yo_60 = -(pcfRadius_48);
          while (true) {
            if ((yo_60 > pcfRadius_48)) {
              break;
            };
            n_53 += 1.0;
            vec2 tmpvar_61;
            tmpvar_61.x = float(xo_52);
            tmpvar_61.y = float(yo_60);
            vec3 tmpvar_62;
            tmpvar_62.xy = (pc_57.xy + (tmpvar_61 * texelSize_55));
            tmpvar_62.z = (pc_57.z - bias_56);
            sumDirect_54 = (sumDirect_54 + texture (shadowMap, tmpvar_62));
            yo_60++;
          };
          xo_52++;
        };
        highp float tmpvar_63;
        tmpvar_63 = (sumDirect_54 / n_53);
        sumIndirect_51 = 0.0;
        sampleScale_50 = mix (4.0, 1.0, tmpvar_63);
        n_53 = 0.0;
        xo_49 = -(pcfRadius_48);
        while (true) {
          highp int yo_64;
          if ((xo_49 > pcfRadius_48)) {
            break;
          };
          yo_64 = -(pcfRadius_48);
          while (true) {
            if ((yo_64 > pcfRadius_48)) {
              break;
            };
            n_53 += 1.0;
            vec2 tmpvar_65;
            tmpvar_65.x = float(xo_49);
            tmpvar_65.y = float(yo_64);
            highp vec3 tmpvar_66;
            tmpvar_66.xy = (pc_57.xy + ((tmpvar_65 * texelSize_55) * sampleScale_50));
            tmpvar_66.z = (pc_57.z - (bias_56 * sampleScale_50));
            sumIndirect_51 = (sumIndirect_51 + texture (shadowMapDetail, tmpvar_66));
            yo_64++;
          };
          xo_49++;
        };
        highp float tmpvar_67;
        tmpvar_67 = (sumIndirect_51 / n_53);
        highp vec2 tmpvar_68;
        tmpvar_68.x = (tmpvar_63 * tmpvar_67);
        tmpvar_68.y = mix (((tmpvar_67 / 2.0) + 0.5), 1.0, tmpvar_63);
        shadow_4 = mix (tmpvar_45, tmpvar_68, tmpvar_21);
        blended_17 = bool(1);
        break;
      };
    };
    if (!(blended_17)) {
      highp int i_69;
      highp int idx_70;
      idx_70 = 0;
      i_69 = 0;
      if ((v_clipPosition.z <= tmpvar_13[0])) {
        idx_70 = i_69;
      } else {
        i_69 = 1;
        if ((v_clipPosition.z <= tmpvar_13[1])) {
          idx_70 = i_69;
        } else {
          i_69 = 2;
          if ((v_clipPosition.z <= tmpvar_13[2])) {
            idx_70 = i_69;
          } else {
            i_69 = 3;
            if ((v_clipPosition.z <= tmpvar_13[3])) {
              idx_70 = i_69;
            } else {
              i_69 = 4;
            };
          };
        };
      };
      float shadowDistance_71;
      shadowDistance_71 = tmpvar_14[idx_70];
      vec4 shadowCoords_72;
      shadowCoords_72 = tmpvar_15[idx_70];
      highp int pcfRadius_73;
      pcfRadius_73 = int[4](2, 1, 1, 1)[idx_70];
      highp float sampleScale_75;
      highp float sumIndirect_76;
      float n_78;
      highp float sumDirect_79;
      vec2 texelSize_80;
      float bias_81;
      vec3 pc_82;
      pc_82 = (((
        (shadowCoords_72 / shadowCoords_72.w)
      .xyz * 0.5) + 0.5) * vec3(0.5, 0.5, 1.0));
      vec3 tmpvar_83;
      tmpvar_83.z = 0.0;
      tmpvar_83.xy = (vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[idx_70] * vec2(0.5, 0.5));
      pc_82 = (pc_82 + tmpvar_83);
      float tmpvar_84;
      tmpvar_84 = float(shadowMapSize);
      bias_81 = (((
        (float[4](1.0, 1.0, 1.0, 1.0)[idx_70] * ((2.0 * shadowDistance_71) / tmpvar_84))
       * 
        (1.0 + (1.0 - clamp (dot (
          normalize(v_normal)
        , 
          normalize(lightDirection)
        ), 0.0, 1.0)))
      ) / (shadowDistance_71 + 1000.0)) * (float(pcfRadius_73) + 1.0));
      texelSize_80 = (0.5 / vec2(tmpvar_84));
      sumDirect_79 = 0.0;
      n_78 = 0.0;
      for (highp int xo_77 = -(pcfRadius_73); xo_77 <= pcfRadius_73; xo_77++) {
        for (highp int yo_85 = -(pcfRadius_73); yo_85 <= pcfRadius_73; yo_85++) {
          n_78 += 1.0;
          vec2 tmpvar_86;
          tmpvar_86.x = float(xo_77);
          tmpvar_86.y = float(yo_85);
          vec3 tmpvar_87;
          tmpvar_87.xy = (pc_82.xy + (tmpvar_86 * texelSize_80));
          tmpvar_87.z = (pc_82.z - bias_81);
          sumDirect_79 = (sumDirect_79 + texture (shadowMap, tmpvar_87));
        };
      };
      highp float tmpvar_88;
      tmpvar_88 = (sumDirect_79 / n_78);
      sumIndirect_76 = 0.0;
      sampleScale_75 = mix (4.0, 1.0, tmpvar_88);
      n_78 = 0.0;
      for (highp int xo_74 = -(pcfRadius_73); xo_74 <= pcfRadius_73; xo_74++) {
        for (highp int yo_89 = -(pcfRadius_73); yo_89 <= pcfRadius_73; yo_89++) {
          n_78 += 1.0;
          vec2 tmpvar_90;
          tmpvar_90.x = float(xo_74);
          tmpvar_90.y = float(yo_89);
          highp vec3 tmpvar_91;
          tmpvar_91.xy = (pc_82.xy + ((tmpvar_90 * texelSize_80) * sampleScale_75));
          tmpvar_91.z = (pc_82.z - (bias_81 * sampleScale_75));
          sumIndirect_76 = (sumIndirect_76 + texture (shadowMapDetail, tmpvar_91));
        };
      };
      highp float tmpvar_92;
      tmpvar_92 = (sumIndirect_76 / n_78);
      highp vec2 tmpvar_93;
      tmpvar_93.x = (tmpvar_88 * tmpvar_92);
      tmpvar_93.y = mix (((tmpvar_92 / 2.0) + 0.5), 1.0, tmpvar_88);
      shadow_4 = tmpvar_93;
    };
  };
  directionPower_10 = (tmpvar_11 * shadow_4.x);
  indirectPower_9 = (tmpvar_12 * shadow_4.y);
  highp vec3 tmpvar_94;
  tmpvar_94 = mix (vec3(indirectPower_9), vec3(1.0, 1.0, 1.0), directionPower_10);
  color_1 = (tmpvar_2.xyz * tmpvar_94);
  lowp vec4 tmpvar_95;
  tmpvar_95.xyz = color_1;
  tmpvar_95.w = tmpvar_3;
  FragColor = tmpvar_95;
}

