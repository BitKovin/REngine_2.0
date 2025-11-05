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
  lowp float alpha_1;
  lowp vec3 color_2;
  lowp vec4 tmpvar_3;
  tmpvar_3 = (texture (u_texture, v_texcoord) * v_color);
  color_2 = tmpvar_3.xyz;
  alpha_1 = tmpvar_3.w;
  if ((tmpvar_3.w < 0.01)) {
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
  indirectPower_9 = mix (0.2, 0.35, indirectPower_9);
  dist_8 = v_clipPosition.z;
  float tmpvar_12[4];
  tmpvar_12[0] = shadowDistance1;
  tmpvar_12[1] = shadowDistance2;
  tmpvar_12[2] = shadowDistance3;
  tmpvar_12[3] = shadowDistance4;
  cascadeDist_7 = tmpvar_12;
  float tmpvar_13[4];
  tmpvar_13[0] = shadowRadius1;
  tmpvar_13[1] = shadowRadius2;
  tmpvar_13[2] = shadowRadius3;
  tmpvar_13[3] = shadowRadius4;
  cascadeRadius_6 = tmpvar_13;
  vec4 tmpvar_14[4];
  tmpvar_14[0] = v_shadowCoords1;
  tmpvar_14[1] = v_shadowCoords2;
  tmpvar_14[2] = v_shadowCoords3;
  tmpvar_14[3] = v_shadowCoords4;
  cascadeCoords_5 = tmpvar_14;
  shadow_4 = vec2(1.0, 1.0);
  if ((v_clipPosition.z < tmpvar_12[3])) {
    bool blended_16;
    blended_16 = bool(0);
    for (highp int i_15 = 0; i_15 < 3; i_15++) {
      float tmpvar_17;
      if ((i_15 == 0)) {
        tmpvar_17 = 0.0;
      } else {
        tmpvar_17 = cascadeDist_7[(i_15 - 1)];
      };
      float tmpvar_18;
      tmpvar_18 = (cascadeDist_7[i_15] - ((cascadeDist_7[i_15] - tmpvar_17) * 0.15));
      float tmpvar_19;
      tmpvar_19 = cascadeDist_7[i_15];
      if (((dist_8 >= tmpvar_18) && (dist_8 <= tmpvar_19))) {
        float blendF_20;
        float tmpvar_21;
        tmpvar_21 = clamp (((dist_8 - tmpvar_18) / (tmpvar_19 - tmpvar_18)), 0.0, 1.0);
        blendF_20 = (tmpvar_21 * (tmpvar_21 * (3.0 - 
          (2.0 * tmpvar_21)
        )));
        vec2 mapOffset_22;
        mapOffset_22 = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[i_15];
        float shadowDistance_23;
        shadowDistance_23 = cascadeRadius_6[i_15];
        highp int pcfRadius_24;
        pcfRadius_24 = int[4](2, 1, 1, 1)[i_15];
        vec4 sc_25;
        sc_25 = cascadeCoords_5[i_15];
        vec3 pc_26;
        vec3 tmpvar_27;
        tmpvar_27 = (((sc_25.xyz / sc_25.w) * 0.5) + 0.5);
        pc_26.z = tmpvar_27.z;
        pc_26.xy = ((tmpvar_27.xy * 0.5) + (mapOffset_22 * 0.5));
        float tmpvar_28;
        float tmpvar_29;
        tmpvar_29 = float(shadowMapSize);
        tmpvar_28 = (((
          (float[4](1.0, 1.0, 1.0, 1.0)[i_15] * ((2.0 * shadowDistance_23) / tmpvar_29))
         * 
          (1.0 + (1.0 - clamp (dot (
            normalize(v_normal)
          , 
            normalize(lightDirection)
          ), 0.0, 1.0)))
        ) / (shadowDistance_23 + 1000.0)) * (float(pcfRadius_24) + 1.0));
        vec2 tmpvar_30;
        tmpvar_30 = (vec2(0.5, 0.5) / tmpvar_29);
        vec2 tmpvar_31;
        tmpvar_31 = (mapOffset_22 * 0.5);
        vec2 tmpvar_32;
        tmpvar_32 = (tmpvar_31 + vec2(0.5, 0.5));
        float tmpvar_33;
        vec3 pc_34;
        pc_34 = pc_26;
        vec2 texelSize_35;
        texelSize_35 = tmpvar_30;
        float bias_36;
        bias_36 = tmpvar_28;
        highp int radius_37;
        radius_37 = pcfRadius_24;
        vec2 tileMin_38;
        tileMin_38 = tmpvar_31;
        vec2 tileMax_39;
        tileMax_39 = tmpvar_32;
        highp int xo_40;
        float n_41;
        highp float sum_42;
        sum_42 = 0.0;
        n_41 = 0.0;
        xo_40 = -(pcfRadius_24);
        while (true) {
          highp int yo_43;
          if ((xo_40 > radius_37)) {
            break;
          };
          yo_43 = -(radius_37);
          while (true) {
            if ((yo_43 > radius_37)) {
              break;
            };
            n_41 += 1.0;
            vec2 tmpvar_44;
            tmpvar_44.x = float(xo_40);
            tmpvar_44.y = float(yo_43);
            vec3 tmpvar_45;
            tmpvar_45.xy = clamp ((pc_34.xy + (tmpvar_44 * texelSize_35)), tileMin_38, tileMax_39);
            tmpvar_45.z = (pc_34.z - bias_36);
            sum_42 = (sum_42 + texture (shadowMap, tmpvar_45));
            yo_43++;
          };
          xo_40++;
        };
        tmpvar_33 = (sum_42 / n_41);
        vec3 pc_46;
        pc_46 = pc_26;
        vec2 texelSize_47;
        texelSize_47 = (tmpvar_30 * mix (4.0, 1.0, tmpvar_33));
        float bias_48;
        bias_48 = tmpvar_28;
        highp int radius_49;
        radius_49 = (int(ceil(
          float(pcfRadius_24)
        )) / 3);
        vec2 tileMin_50;
        tileMin_50 = tmpvar_31;
        vec2 tileMax_51;
        tileMax_51 = tmpvar_32;
        highp int xo_52;
        float n_53;
        highp float sum_54;
        sum_54 = 0.0;
        n_53 = 0.0;
        xo_52 = -(radius_49);
        while (true) {
          highp int yo_55;
          if ((xo_52 > radius_49)) {
            break;
          };
          yo_55 = -(radius_49);
          while (true) {
            if ((yo_55 > radius_49)) {
              break;
            };
            n_53 += 1.0;
            vec2 tmpvar_56;
            tmpvar_56.x = float(xo_52);
            tmpvar_56.y = float(yo_55);
            vec3 tmpvar_57;
            tmpvar_57.xy = clamp ((pc_46.xy + (tmpvar_56 * texelSize_47)), tileMin_50, tileMax_51);
            tmpvar_57.z = (pc_46.z - bias_48);
            sum_54 = (sum_54 + texture (shadowMapDetail, tmpvar_57));
            yo_55++;
          };
          xo_52++;
        };
        highp float tmpvar_58;
        tmpvar_58 = mix ((sum_54 / n_53), 1.0, 0.5);
        highp vec2 tmpvar_59;
        tmpvar_59.x = (tmpvar_33 * tmpvar_58);
        tmpvar_59.y = mix (((tmpvar_58 * 0.5) + 0.5), 1.0, tmpvar_33);
        vec2 mapOffset_60;
        mapOffset_60 = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[(i_15 + 1)];
        float shadowDistance_61;
        shadowDistance_61 = cascadeRadius_6[(i_15 + 1)];
        highp int pcfRadius_62;
        pcfRadius_62 = int[4](2, 1, 1, 1)[(i_15 + 1)];
        vec4 sc_63;
        sc_63 = cascadeCoords_5[(i_15 + 1)];
        vec3 pc_64;
        vec3 tmpvar_65;
        tmpvar_65 = (((sc_63.xyz / sc_63.w) * 0.5) + 0.5);
        pc_64.z = tmpvar_65.z;
        pc_64.xy = ((tmpvar_65.xy * 0.5) + (mapOffset_60 * 0.5));
        float tmpvar_66;
        tmpvar_66 = (((
          (float[4](1.0, 1.0, 1.0, 1.0)[(i_15 + 1)] * ((2.0 * shadowDistance_61) / float(shadowMapSize)))
         * 
          (1.0 + (1.0 - clamp (dot (
            normalize(v_normal)
          , 
            normalize(lightDirection)
          ), 0.0, 1.0)))
        ) / (shadowDistance_61 + 1000.0)) * (float(pcfRadius_62) + 1.0));
        vec2 tmpvar_67;
        tmpvar_67 = (vec2(0.5, 0.5) / tmpvar_29);
        vec2 tmpvar_68;
        tmpvar_68 = (mapOffset_60 * 0.5);
        vec2 tmpvar_69;
        tmpvar_69 = (tmpvar_68 + vec2(0.5, 0.5));
        float tmpvar_70;
        vec3 pc_71;
        pc_71 = pc_64;
        vec2 texelSize_72;
        texelSize_72 = tmpvar_67;
        float bias_73;
        bias_73 = tmpvar_66;
        highp int radius_74;
        radius_74 = pcfRadius_62;
        vec2 tileMin_75;
        tileMin_75 = tmpvar_68;
        vec2 tileMax_76;
        tileMax_76 = tmpvar_69;
        highp int xo_77;
        float n_78;
        highp float sum_79;
        sum_79 = 0.0;
        n_78 = 0.0;
        xo_77 = -(pcfRadius_62);
        while (true) {
          highp int yo_80;
          if ((xo_77 > radius_74)) {
            break;
          };
          yo_80 = -(radius_74);
          while (true) {
            if ((yo_80 > radius_74)) {
              break;
            };
            n_78 += 1.0;
            vec2 tmpvar_81;
            tmpvar_81.x = float(xo_77);
            tmpvar_81.y = float(yo_80);
            vec3 tmpvar_82;
            tmpvar_82.xy = clamp ((pc_71.xy + (tmpvar_81 * texelSize_72)), tileMin_75, tileMax_76);
            tmpvar_82.z = (pc_71.z - bias_73);
            sum_79 = (sum_79 + texture (shadowMap, tmpvar_82));
            yo_80++;
          };
          xo_77++;
        };
        tmpvar_70 = (sum_79 / n_78);
        vec3 pc_83;
        pc_83 = pc_64;
        vec2 texelSize_84;
        texelSize_84 = (tmpvar_67 * mix (4.0, 1.0, tmpvar_70));
        float bias_85;
        bias_85 = tmpvar_66;
        highp int radius_86;
        radius_86 = (int(ceil(
          float(pcfRadius_62)
        )) / 3);
        vec2 tileMin_87;
        tileMin_87 = tmpvar_68;
        vec2 tileMax_88;
        tileMax_88 = tmpvar_69;
        highp int xo_89;
        float n_90;
        highp float sum_91;
        sum_91 = 0.0;
        n_90 = 0.0;
        xo_89 = -(radius_86);
        while (true) {
          highp int yo_92;
          if ((xo_89 > radius_86)) {
            break;
          };
          yo_92 = -(radius_86);
          while (true) {
            if ((yo_92 > radius_86)) {
              break;
            };
            n_90 += 1.0;
            vec2 tmpvar_93;
            tmpvar_93.x = float(xo_89);
            tmpvar_93.y = float(yo_92);
            vec3 tmpvar_94;
            tmpvar_94.xy = clamp ((pc_83.xy + (tmpvar_93 * texelSize_84)), tileMin_87, tileMax_88);
            tmpvar_94.z = (pc_83.z - bias_85);
            sum_91 = (sum_91 + texture (shadowMapDetail, tmpvar_94));
            yo_92++;
          };
          xo_89++;
        };
        highp float tmpvar_95;
        tmpvar_95 = mix ((sum_91 / n_90), 1.0, 0.5);
        highp vec2 tmpvar_96;
        tmpvar_96.x = (tmpvar_70 * tmpvar_95);
        tmpvar_96.y = mix (((tmpvar_95 * 0.5) + 0.5), 1.0, tmpvar_70);
        shadow_4 = mix (tmpvar_59, tmpvar_96, blendF_20);
        blended_16 = bool(1);
        break;
      };
    };
    if (!(blended_16)) {
      highp int i_97;
      highp int idx_98;
      idx_98 = 0;
      i_97 = 0;
      if ((v_clipPosition.z <= tmpvar_12[0])) {
        idx_98 = i_97;
      } else {
        i_97 = 1;
        if ((v_clipPosition.z <= tmpvar_12[1])) {
          idx_98 = i_97;
        } else {
          i_97 = 2;
          if ((v_clipPosition.z <= tmpvar_12[2])) {
            idx_98 = i_97;
          } else {
            i_97 = 3;
            if ((v_clipPosition.z <= tmpvar_12[3])) {
              idx_98 = i_97;
            } else {
              i_97 = 4;
            };
          };
        };
      };
      vec2 mapOffset_99;
      mapOffset_99 = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[idx_98];
      float shadowDistance_100;
      shadowDistance_100 = tmpvar_13[idx_98];
      highp int pcfRadius_101;
      pcfRadius_101 = int[4](2, 1, 1, 1)[idx_98];
      vec4 sc_102;
      sc_102 = tmpvar_14[idx_98];
      vec3 pc_103;
      vec3 tmpvar_104;
      tmpvar_104 = (((sc_102.xyz / sc_102.w) * 0.5) + 0.5);
      pc_103.z = tmpvar_104.z;
      pc_103.xy = ((tmpvar_104.xy * 0.5) + (mapOffset_99 * 0.5));
      float tmpvar_105;
      float tmpvar_106;
      tmpvar_106 = float(shadowMapSize);
      tmpvar_105 = (((
        (float[4](1.0, 1.0, 1.0, 1.0)[idx_98] * ((2.0 * shadowDistance_100) / tmpvar_106))
       * 
        (1.0 + (1.0 - clamp (dot (
          normalize(v_normal)
        , 
          normalize(lightDirection)
        ), 0.0, 1.0)))
      ) / (shadowDistance_100 + 1000.0)) * (float(pcfRadius_101) + 1.0));
      vec2 tmpvar_107;
      tmpvar_107 = (vec2(0.5, 0.5) / tmpvar_106);
      vec2 tmpvar_108;
      tmpvar_108 = (mapOffset_99 * 0.5);
      vec2 tmpvar_109;
      tmpvar_109 = (tmpvar_108 + vec2(0.5, 0.5));
      float tmpvar_110;
      vec3 pc_111;
      pc_111 = pc_103;
      vec2 texelSize_112;
      texelSize_112 = tmpvar_107;
      float bias_113;
      bias_113 = tmpvar_105;
      highp int radius_114;
      radius_114 = pcfRadius_101;
      vec2 tileMin_115;
      tileMin_115 = tmpvar_108;
      vec2 tileMax_116;
      tileMax_116 = tmpvar_109;
      float n_118;
      highp float sum_119;
      sum_119 = 0.0;
      n_118 = 0.0;
      for (highp int xo_117 = -(pcfRadius_101); xo_117 <= radius_114; xo_117++) {
        for (highp int yo_120 = -(radius_114); yo_120 <= radius_114; yo_120++) {
          n_118 += 1.0;
          vec2 tmpvar_121;
          tmpvar_121.x = float(xo_117);
          tmpvar_121.y = float(yo_120);
          vec3 tmpvar_122;
          tmpvar_122.xy = clamp ((pc_111.xy + (tmpvar_121 * texelSize_112)), tileMin_115, tileMax_116);
          tmpvar_122.z = (pc_111.z - bias_113);
          sum_119 = (sum_119 + texture (shadowMap, tmpvar_122));
        };
      };
      tmpvar_110 = (sum_119 / n_118);
      vec3 pc_123;
      pc_123 = pc_103;
      vec2 texelSize_124;
      texelSize_124 = (tmpvar_107 * mix (4.0, 1.0, tmpvar_110));
      float bias_125;
      bias_125 = tmpvar_105;
      highp int radius_126;
      radius_126 = (int(ceil(
        float(pcfRadius_101)
      )) / 3);
      vec2 tileMin_127;
      tileMin_127 = tmpvar_108;
      vec2 tileMax_128;
      tileMax_128 = tmpvar_109;
      float n_130;
      highp float sum_131;
      sum_131 = 0.0;
      n_130 = 0.0;
      for (highp int xo_129 = -(radius_126); xo_129 <= radius_126; xo_129++) {
        for (highp int yo_132 = -(radius_126); yo_132 <= radius_126; yo_132++) {
          n_130 += 1.0;
          vec2 tmpvar_133;
          tmpvar_133.x = float(xo_129);
          tmpvar_133.y = float(yo_132);
          vec3 tmpvar_134;
          tmpvar_134.xy = clamp ((pc_123.xy + (tmpvar_133 * texelSize_124)), tileMin_127, tileMax_128);
          tmpvar_134.z = (pc_123.z - bias_125);
          sum_131 = (sum_131 + texture (shadowMapDetail, tmpvar_134));
        };
      };
      highp float tmpvar_135;
      tmpvar_135 = mix ((sum_131 / n_130), 1.0, 0.5);
      highp vec2 tmpvar_136;
      tmpvar_136.x = (tmpvar_110 * tmpvar_135);
      tmpvar_136.y = mix (((tmpvar_135 * 0.5) + 0.5), 1.0, tmpvar_110);
      shadow_4 = tmpvar_136;
    };
  };
  directionPower_10 = (tmpvar_11 * shadow_4.x);
  indirectPower_9 = (indirectPower_9 * shadow_4.y);
  color_2 = (tmpvar_3.xyz * mix (vec3(indirectPower_9), vec3(1.0, 1.0, 1.0), directionPower_10));
  lowp vec4 tmpvar_137;
  tmpvar_137.xyz = color_2;
  tmpvar_137.w = alpha_1;
  FragColor = tmpvar_137;
}

