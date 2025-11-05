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
uniform highp sampler2D shadowMapDetailRaw;
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
        float tmpvar_46;
        tmpvar_46 = mix (200.0, 40.0, tmpvar_33);
        float tmpvar_47;
        tmpvar_47 = mix (5.0, 50.0, tmpvar_33);
        vec3 pc_48;
        pc_48 = pc_26;
        vec2 texelSize_49;
        texelSize_49 = tmpvar_30;
        float baseBias_50;
        baseBias_50 = tmpvar_28;
        vec2 tileMin_51;
        tileMin_51 = tmpvar_31;
        vec2 tileMax_52;
        tileMax_52 = tmpvar_32;
        highp float tmpvar_53;
        lowp int xo_54;
        highp int cntS_55;
        highp float sumS_56;
        lowp float dynBias_57;
        lowp int r_58;
        highp int xo_59;
        highp int cntB_60;
        lowp float sumB_61;
        sumB_61 = 0.0;
        cntB_60 = 0;
        xo_59 = -3;
        while (true) {
          if ((xo_59 > 3)) {
            break;
          };
          vec2 tmpvar_62;
          tmpvar_62.x = float(xo_59);
          tmpvar_62.y = -3.0;
          vec2 tmpvar_63;
          tmpvar_63.x = float(xo_59);
          tmpvar_63.y = -3.0;
          lowp vec4 tmpvar_64;
          tmpvar_64 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_62 * texelSize_49) * sqrt(dot (tmpvar_63, tmpvar_63)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_64.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_64.x);
            cntB_60++;
          };
          vec2 tmpvar_65;
          tmpvar_65.x = float(xo_59);
          tmpvar_65.y = -2.0;
          vec2 tmpvar_66;
          tmpvar_66.x = float(xo_59);
          tmpvar_66.y = -2.0;
          lowp vec4 tmpvar_67;
          tmpvar_67 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_65 * texelSize_49) * sqrt(dot (tmpvar_66, tmpvar_66)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_67.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_67.x);
            cntB_60++;
          };
          vec2 tmpvar_68;
          tmpvar_68.x = float(xo_59);
          tmpvar_68.y = -1.0;
          vec2 tmpvar_69;
          tmpvar_69.x = float(xo_59);
          tmpvar_69.y = -1.0;
          lowp vec4 tmpvar_70;
          tmpvar_70 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_68 * texelSize_49) * sqrt(dot (tmpvar_69, tmpvar_69)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_70.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_70.x);
            cntB_60++;
          };
          vec2 tmpvar_71;
          tmpvar_71.x = float(xo_59);
          tmpvar_71.y = 0.0;
          vec2 tmpvar_72;
          tmpvar_72.x = float(xo_59);
          tmpvar_72.y = 0.0;
          lowp vec4 tmpvar_73;
          tmpvar_73 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_71 * texelSize_49) * sqrt(dot (tmpvar_72, tmpvar_72)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_73.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_73.x);
            cntB_60++;
          };
          vec2 tmpvar_74;
          tmpvar_74.x = float(xo_59);
          tmpvar_74.y = 1.0;
          vec2 tmpvar_75;
          tmpvar_75.x = float(xo_59);
          tmpvar_75.y = 1.0;
          lowp vec4 tmpvar_76;
          tmpvar_76 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_74 * texelSize_49) * sqrt(dot (tmpvar_75, tmpvar_75)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_76.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_76.x);
            cntB_60++;
          };
          vec2 tmpvar_77;
          tmpvar_77.x = float(xo_59);
          tmpvar_77.y = 2.0;
          vec2 tmpvar_78;
          tmpvar_78.x = float(xo_59);
          tmpvar_78.y = 2.0;
          lowp vec4 tmpvar_79;
          tmpvar_79 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_77 * texelSize_49) * sqrt(dot (tmpvar_78, tmpvar_78)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_79.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_79.x);
            cntB_60++;
          };
          vec2 tmpvar_80;
          tmpvar_80.x = float(xo_59);
          tmpvar_80.y = 3.0;
          vec2 tmpvar_81;
          tmpvar_81.x = float(xo_59);
          tmpvar_81.y = 3.0;
          lowp vec4 tmpvar_82;
          tmpvar_82 = texture (shadowMapDetailRaw, clamp ((pc_48.xy + (
            ((tmpvar_80 * texelSize_49) * sqrt(dot (tmpvar_81, tmpvar_81)))
           * 5.0)), tileMin_51, tileMax_52));
          if ((tmpvar_82.x < (pc_48.z - baseBias_50))) {
            sumB_61 = (sumB_61 + tmpvar_82.x);
            cntB_60++;
          };
          xo_59++;
        };
        if ((cntB_60 == 0)) {
          tmpvar_53 = 1.0;
        } else {
          float tmpvar_83;
          tmpvar_83 = (shadowDistance_23 + 1000.0);
          float tmpvar_84;
          tmpvar_84 = (tmpvar_27.z * tmpvar_83);
          lowp float tmpvar_85;
          tmpvar_85 = ((sumB_61 / float(cntB_60)) * tmpvar_83);
          lowp int tmpvar_86;
          tmpvar_86 = int(ceil(clamp (
            ((((tmpvar_46 * 
              (tmpvar_84 - tmpvar_85)
            ) / tmpvar_85) / (2.0 * shadowDistance_23)) * float(shadowMapSize))
          , 2.0, 
            (float(pcfRadius_24) * 8.0)
          )));
          r_58 = tmpvar_86;
          dynBias_57 = (tmpvar_28 * max ((
            (tmpvar_84 - tmpvar_85)
           / tmpvar_85), 1.0));
          sumS_56 = 0.0;
          cntS_55 = 0;
          xo_54 = -(tmpvar_86);
          while (true) {
            lowp int yo_87;
            if ((xo_54 > r_58)) {
              break;
            };
            yo_87 = -(r_58);
            while (true) {
              if ((yo_87 > r_58)) {
                break;
              };
              lowp vec2 tmpvar_88;
              tmpvar_88.x = float(xo_54);
              tmpvar_88.y = float(yo_87);
              lowp vec3 tmpvar_89;
              tmpvar_89.xy = clamp ((pc_48.xy + (tmpvar_88 * texelSize_49)), tileMin_51, tileMax_52);
              tmpvar_89.z = (pc_48.z - dynBias_57);
              sumS_56 = (sumS_56 + texture (shadowMapDetail, tmpvar_89));
              cntS_55++;
              yo_87++;
            };
            xo_54++;
          };
          lowp float tmpvar_90;
          lowp float tmpvar_91;
          tmpvar_91 = clamp (clamp ((
            (tmpvar_84 - tmpvar_85)
           / tmpvar_47), 0.0, 1.0), 0.0, 1.0);
          tmpvar_90 = (tmpvar_91 * (tmpvar_91 * (3.0 - 
            (2.0 * tmpvar_91)
          )));
          tmpvar_53 = mix ((sumS_56 / float(cntS_55)), 1.0, tmpvar_90);
        };
        highp vec2 tmpvar_92;
        tmpvar_92.x = (tmpvar_33 * tmpvar_53);
        tmpvar_92.y = mix (((tmpvar_53 * 0.7) + 0.3), 1.0, tmpvar_33);
        vec2 mapOffset_93;
        mapOffset_93 = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[(i_15 + 1)];
        float shadowDistance_94;
        shadowDistance_94 = cascadeRadius_6[(i_15 + 1)];
        highp int pcfRadius_95;
        pcfRadius_95 = int[4](2, 1, 1, 1)[(i_15 + 1)];
        vec4 sc_96;
        sc_96 = cascadeCoords_5[(i_15 + 1)];
        vec3 pc_97;
        vec3 tmpvar_98;
        tmpvar_98 = (((sc_96.xyz / sc_96.w) * 0.5) + 0.5);
        pc_97.z = tmpvar_98.z;
        pc_97.xy = ((tmpvar_98.xy * 0.5) + (mapOffset_93 * 0.5));
        float tmpvar_99;
        tmpvar_99 = (((
          (float[4](1.0, 1.0, 1.0, 1.0)[(i_15 + 1)] * ((2.0 * shadowDistance_94) / float(shadowMapSize)))
         * 
          (1.0 + (1.0 - clamp (dot (
            normalize(v_normal)
          , 
            normalize(lightDirection)
          ), 0.0, 1.0)))
        ) / (shadowDistance_94 + 1000.0)) * (float(pcfRadius_95) + 1.0));
        vec2 tmpvar_100;
        tmpvar_100 = (vec2(0.5, 0.5) / tmpvar_29);
        vec2 tmpvar_101;
        tmpvar_101 = (mapOffset_93 * 0.5);
        vec2 tmpvar_102;
        tmpvar_102 = (tmpvar_101 + vec2(0.5, 0.5));
        float tmpvar_103;
        vec3 pc_104;
        pc_104 = pc_97;
        vec2 texelSize_105;
        texelSize_105 = tmpvar_100;
        float bias_106;
        bias_106 = tmpvar_99;
        highp int radius_107;
        radius_107 = pcfRadius_95;
        vec2 tileMin_108;
        tileMin_108 = tmpvar_101;
        vec2 tileMax_109;
        tileMax_109 = tmpvar_102;
        highp int xo_110;
        float n_111;
        highp float sum_112;
        sum_112 = 0.0;
        n_111 = 0.0;
        xo_110 = -(pcfRadius_95);
        while (true) {
          highp int yo_113;
          if ((xo_110 > radius_107)) {
            break;
          };
          yo_113 = -(radius_107);
          while (true) {
            if ((yo_113 > radius_107)) {
              break;
            };
            n_111 += 1.0;
            vec2 tmpvar_114;
            tmpvar_114.x = float(xo_110);
            tmpvar_114.y = float(yo_113);
            vec3 tmpvar_115;
            tmpvar_115.xy = clamp ((pc_104.xy + (tmpvar_114 * texelSize_105)), tileMin_108, tileMax_109);
            tmpvar_115.z = (pc_104.z - bias_106);
            sum_112 = (sum_112 + texture (shadowMap, tmpvar_115));
            yo_113++;
          };
          xo_110++;
        };
        tmpvar_103 = (sum_112 / n_111);
        float tmpvar_116;
        tmpvar_116 = mix (200.0, 40.0, tmpvar_103);
        float tmpvar_117;
        tmpvar_117 = mix (5.0, 50.0, tmpvar_103);
        vec3 pc_118;
        pc_118 = pc_97;
        vec2 texelSize_119;
        texelSize_119 = tmpvar_100;
        float baseBias_120;
        baseBias_120 = tmpvar_99;
        vec2 tileMin_121;
        tileMin_121 = tmpvar_101;
        vec2 tileMax_122;
        tileMax_122 = tmpvar_102;
        highp float tmpvar_123;
        lowp int xo_124;
        highp int cntS_125;
        highp float sumS_126;
        lowp float dynBias_127;
        lowp int r_128;
        highp int xo_129;
        highp int cntB_130;
        lowp float sumB_131;
        sumB_131 = 0.0;
        cntB_130 = 0;
        xo_129 = -3;
        while (true) {
          if ((xo_129 > 3)) {
            break;
          };
          vec2 tmpvar_132;
          tmpvar_132.x = float(xo_129);
          tmpvar_132.y = -3.0;
          vec2 tmpvar_133;
          tmpvar_133.x = float(xo_129);
          tmpvar_133.y = -3.0;
          lowp vec4 tmpvar_134;
          tmpvar_134 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_132 * texelSize_119) * sqrt(dot (tmpvar_133, tmpvar_133)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_134.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_134.x);
            cntB_130++;
          };
          vec2 tmpvar_135;
          tmpvar_135.x = float(xo_129);
          tmpvar_135.y = -2.0;
          vec2 tmpvar_136;
          tmpvar_136.x = float(xo_129);
          tmpvar_136.y = -2.0;
          lowp vec4 tmpvar_137;
          tmpvar_137 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_135 * texelSize_119) * sqrt(dot (tmpvar_136, tmpvar_136)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_137.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_137.x);
            cntB_130++;
          };
          vec2 tmpvar_138;
          tmpvar_138.x = float(xo_129);
          tmpvar_138.y = -1.0;
          vec2 tmpvar_139;
          tmpvar_139.x = float(xo_129);
          tmpvar_139.y = -1.0;
          lowp vec4 tmpvar_140;
          tmpvar_140 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_138 * texelSize_119) * sqrt(dot (tmpvar_139, tmpvar_139)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_140.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_140.x);
            cntB_130++;
          };
          vec2 tmpvar_141;
          tmpvar_141.x = float(xo_129);
          tmpvar_141.y = 0.0;
          vec2 tmpvar_142;
          tmpvar_142.x = float(xo_129);
          tmpvar_142.y = 0.0;
          lowp vec4 tmpvar_143;
          tmpvar_143 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_141 * texelSize_119) * sqrt(dot (tmpvar_142, tmpvar_142)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_143.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_143.x);
            cntB_130++;
          };
          vec2 tmpvar_144;
          tmpvar_144.x = float(xo_129);
          tmpvar_144.y = 1.0;
          vec2 tmpvar_145;
          tmpvar_145.x = float(xo_129);
          tmpvar_145.y = 1.0;
          lowp vec4 tmpvar_146;
          tmpvar_146 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_144 * texelSize_119) * sqrt(dot (tmpvar_145, tmpvar_145)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_146.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_146.x);
            cntB_130++;
          };
          vec2 tmpvar_147;
          tmpvar_147.x = float(xo_129);
          tmpvar_147.y = 2.0;
          vec2 tmpvar_148;
          tmpvar_148.x = float(xo_129);
          tmpvar_148.y = 2.0;
          lowp vec4 tmpvar_149;
          tmpvar_149 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_147 * texelSize_119) * sqrt(dot (tmpvar_148, tmpvar_148)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_149.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_149.x);
            cntB_130++;
          };
          vec2 tmpvar_150;
          tmpvar_150.x = float(xo_129);
          tmpvar_150.y = 3.0;
          vec2 tmpvar_151;
          tmpvar_151.x = float(xo_129);
          tmpvar_151.y = 3.0;
          lowp vec4 tmpvar_152;
          tmpvar_152 = texture (shadowMapDetailRaw, clamp ((pc_118.xy + (
            ((tmpvar_150 * texelSize_119) * sqrt(dot (tmpvar_151, tmpvar_151)))
           * 5.0)), tileMin_121, tileMax_122));
          if ((tmpvar_152.x < (pc_118.z - baseBias_120))) {
            sumB_131 = (sumB_131 + tmpvar_152.x);
            cntB_130++;
          };
          xo_129++;
        };
        if ((cntB_130 == 0)) {
          tmpvar_123 = 1.0;
        } else {
          float tmpvar_153;
          tmpvar_153 = (shadowDistance_94 + 1000.0);
          float tmpvar_154;
          tmpvar_154 = (tmpvar_98.z * tmpvar_153);
          lowp float tmpvar_155;
          tmpvar_155 = ((sumB_131 / float(cntB_130)) * tmpvar_153);
          lowp int tmpvar_156;
          tmpvar_156 = int(ceil(clamp (
            ((((tmpvar_116 * 
              (tmpvar_154 - tmpvar_155)
            ) / tmpvar_155) / (2.0 * shadowDistance_94)) * float(shadowMapSize))
          , 2.0, 
            (float(pcfRadius_95) * 8.0)
          )));
          r_128 = tmpvar_156;
          dynBias_127 = (tmpvar_99 * max ((
            (tmpvar_154 - tmpvar_155)
           / tmpvar_155), 1.0));
          sumS_126 = 0.0;
          cntS_125 = 0;
          xo_124 = -(tmpvar_156);
          while (true) {
            lowp int yo_157;
            if ((xo_124 > r_128)) {
              break;
            };
            yo_157 = -(r_128);
            while (true) {
              if ((yo_157 > r_128)) {
                break;
              };
              lowp vec2 tmpvar_158;
              tmpvar_158.x = float(xo_124);
              tmpvar_158.y = float(yo_157);
              lowp vec3 tmpvar_159;
              tmpvar_159.xy = clamp ((pc_118.xy + (tmpvar_158 * texelSize_119)), tileMin_121, tileMax_122);
              tmpvar_159.z = (pc_118.z - dynBias_127);
              sumS_126 = (sumS_126 + texture (shadowMapDetail, tmpvar_159));
              cntS_125++;
              yo_157++;
            };
            xo_124++;
          };
          lowp float tmpvar_160;
          lowp float tmpvar_161;
          tmpvar_161 = clamp (clamp ((
            (tmpvar_154 - tmpvar_155)
           / tmpvar_117), 0.0, 1.0), 0.0, 1.0);
          tmpvar_160 = (tmpvar_161 * (tmpvar_161 * (3.0 - 
            (2.0 * tmpvar_161)
          )));
          tmpvar_123 = mix ((sumS_126 / float(cntS_125)), 1.0, tmpvar_160);
        };
        highp vec2 tmpvar_162;
        tmpvar_162.x = (tmpvar_103 * tmpvar_123);
        tmpvar_162.y = mix (((tmpvar_123 * 0.7) + 0.3), 1.0, tmpvar_103);
        shadow_4 = mix (tmpvar_92, tmpvar_162, blendF_20);
        blended_16 = bool(1);
        break;
      };
    };
    if (!(blended_16)) {
      highp int i_163;
      highp int idx_164;
      idx_164 = 0;
      i_163 = 0;
      if ((v_clipPosition.z <= tmpvar_12[0])) {
        idx_164 = i_163;
      } else {
        i_163 = 1;
        if ((v_clipPosition.z <= tmpvar_12[1])) {
          idx_164 = i_163;
        } else {
          i_163 = 2;
          if ((v_clipPosition.z <= tmpvar_12[2])) {
            idx_164 = i_163;
          } else {
            i_163 = 3;
            if ((v_clipPosition.z <= tmpvar_12[3])) {
              idx_164 = i_163;
            } else {
              i_163 = 4;
            };
          };
        };
      };
      vec2 mapOffset_165;
      mapOffset_165 = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0))[idx_164];
      float shadowDistance_166;
      shadowDistance_166 = tmpvar_13[idx_164];
      highp int pcfRadius_167;
      pcfRadius_167 = int[4](2, 1, 1, 1)[idx_164];
      vec4 sc_168;
      sc_168 = tmpvar_14[idx_164];
      vec3 pc_169;
      vec3 tmpvar_170;
      tmpvar_170 = (((sc_168.xyz / sc_168.w) * 0.5) + 0.5);
      pc_169.z = tmpvar_170.z;
      pc_169.xy = ((tmpvar_170.xy * 0.5) + (mapOffset_165 * 0.5));
      float tmpvar_171;
      float tmpvar_172;
      tmpvar_172 = float(shadowMapSize);
      tmpvar_171 = (((
        (float[4](1.0, 1.0, 1.0, 1.0)[idx_164] * ((2.0 * shadowDistance_166) / tmpvar_172))
       * 
        (1.0 + (1.0 - clamp (dot (
          normalize(v_normal)
        , 
          normalize(lightDirection)
        ), 0.0, 1.0)))
      ) / (shadowDistance_166 + 1000.0)) * (float(pcfRadius_167) + 1.0));
      vec2 tmpvar_173;
      tmpvar_173 = (vec2(0.5, 0.5) / tmpvar_172);
      vec2 tmpvar_174;
      tmpvar_174 = (mapOffset_165 * 0.5);
      vec2 tmpvar_175;
      tmpvar_175 = (tmpvar_174 + vec2(0.5, 0.5));
      float tmpvar_176;
      vec3 pc_177;
      pc_177 = pc_169;
      vec2 texelSize_178;
      texelSize_178 = tmpvar_173;
      float bias_179;
      bias_179 = tmpvar_171;
      highp int radius_180;
      radius_180 = pcfRadius_167;
      vec2 tileMin_181;
      tileMin_181 = tmpvar_174;
      vec2 tileMax_182;
      tileMax_182 = tmpvar_175;
      float n_184;
      highp float sum_185;
      sum_185 = 0.0;
      n_184 = 0.0;
      for (highp int xo_183 = -(pcfRadius_167); xo_183 <= radius_180; xo_183++) {
        for (highp int yo_186 = -(radius_180); yo_186 <= radius_180; yo_186++) {
          n_184 += 1.0;
          vec2 tmpvar_187;
          tmpvar_187.x = float(xo_183);
          tmpvar_187.y = float(yo_186);
          vec3 tmpvar_188;
          tmpvar_188.xy = clamp ((pc_177.xy + (tmpvar_187 * texelSize_178)), tileMin_181, tileMax_182);
          tmpvar_188.z = (pc_177.z - bias_179);
          sum_185 = (sum_185 + texture (shadowMap, tmpvar_188));
        };
      };
      tmpvar_176 = (sum_185 / n_184);
      float tmpvar_189;
      tmpvar_189 = mix (200.0, 40.0, tmpvar_176);
      float tmpvar_190;
      tmpvar_190 = mix (5.0, 50.0, tmpvar_176);
      vec3 pc_191;
      pc_191 = pc_169;
      vec2 texelSize_192;
      texelSize_192 = tmpvar_173;
      float baseBias_193;
      baseBias_193 = tmpvar_171;
      vec2 tileMin_194;
      tileMin_194 = tmpvar_174;
      vec2 tileMax_195;
      tileMax_195 = tmpvar_175;
      highp float tmpvar_196;
      highp int cntS_198;
      highp float sumS_199;
      lowp float dynBias_200;
      lowp int r_201;
      highp int cntB_203;
      lowp float sumB_204;
      sumB_204 = 0.0;
      cntB_203 = 0;
      for (highp int xo_202 = -3; xo_202 <= 3; xo_202++) {
        vec2 tmpvar_205;
        tmpvar_205.x = float(xo_202);
        tmpvar_205.y = -3.0;
        vec2 tmpvar_206;
        tmpvar_206.x = float(xo_202);
        tmpvar_206.y = -3.0;
        lowp vec4 tmpvar_207;
        tmpvar_207 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_205 * texelSize_192) * sqrt(dot (tmpvar_206, tmpvar_206)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_207.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_207.x);
          cntB_203++;
        };
        vec2 tmpvar_208;
        tmpvar_208.x = float(xo_202);
        tmpvar_208.y = -2.0;
        vec2 tmpvar_209;
        tmpvar_209.x = float(xo_202);
        tmpvar_209.y = -2.0;
        lowp vec4 tmpvar_210;
        tmpvar_210 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_208 * texelSize_192) * sqrt(dot (tmpvar_209, tmpvar_209)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_210.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_210.x);
          cntB_203++;
        };
        vec2 tmpvar_211;
        tmpvar_211.x = float(xo_202);
        tmpvar_211.y = -1.0;
        vec2 tmpvar_212;
        tmpvar_212.x = float(xo_202);
        tmpvar_212.y = -1.0;
        lowp vec4 tmpvar_213;
        tmpvar_213 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_211 * texelSize_192) * sqrt(dot (tmpvar_212, tmpvar_212)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_213.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_213.x);
          cntB_203++;
        };
        vec2 tmpvar_214;
        tmpvar_214.x = float(xo_202);
        tmpvar_214.y = 0.0;
        vec2 tmpvar_215;
        tmpvar_215.x = float(xo_202);
        tmpvar_215.y = 0.0;
        lowp vec4 tmpvar_216;
        tmpvar_216 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_214 * texelSize_192) * sqrt(dot (tmpvar_215, tmpvar_215)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_216.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_216.x);
          cntB_203++;
        };
        vec2 tmpvar_217;
        tmpvar_217.x = float(xo_202);
        tmpvar_217.y = 1.0;
        vec2 tmpvar_218;
        tmpvar_218.x = float(xo_202);
        tmpvar_218.y = 1.0;
        lowp vec4 tmpvar_219;
        tmpvar_219 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_217 * texelSize_192) * sqrt(dot (tmpvar_218, tmpvar_218)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_219.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_219.x);
          cntB_203++;
        };
        vec2 tmpvar_220;
        tmpvar_220.x = float(xo_202);
        tmpvar_220.y = 2.0;
        vec2 tmpvar_221;
        tmpvar_221.x = float(xo_202);
        tmpvar_221.y = 2.0;
        lowp vec4 tmpvar_222;
        tmpvar_222 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_220 * texelSize_192) * sqrt(dot (tmpvar_221, tmpvar_221)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_222.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_222.x);
          cntB_203++;
        };
        vec2 tmpvar_223;
        tmpvar_223.x = float(xo_202);
        tmpvar_223.y = 3.0;
        vec2 tmpvar_224;
        tmpvar_224.x = float(xo_202);
        tmpvar_224.y = 3.0;
        lowp vec4 tmpvar_225;
        tmpvar_225 = texture (shadowMapDetailRaw, clamp ((pc_191.xy + (
          ((tmpvar_223 * texelSize_192) * sqrt(dot (tmpvar_224, tmpvar_224)))
         * 5.0)), tileMin_194, tileMax_195));
        if ((tmpvar_225.x < (pc_191.z - baseBias_193))) {
          sumB_204 = (sumB_204 + tmpvar_225.x);
          cntB_203++;
        };
      };
      if ((cntB_203 == 0)) {
        tmpvar_196 = 1.0;
      } else {
        float tmpvar_226;
        tmpvar_226 = (shadowDistance_166 + 1000.0);
        float tmpvar_227;
        tmpvar_227 = (tmpvar_170.z * tmpvar_226);
        lowp float tmpvar_228;
        tmpvar_228 = ((sumB_204 / float(cntB_203)) * tmpvar_226);
        lowp int tmpvar_229;
        tmpvar_229 = int(ceil(clamp (
          ((((tmpvar_189 * 
            (tmpvar_227 - tmpvar_228)
          ) / tmpvar_228) / (2.0 * shadowDistance_166)) * float(shadowMapSize))
        , 2.0, 
          (float(pcfRadius_167) * 8.0)
        )));
        r_201 = tmpvar_229;
        dynBias_200 = (tmpvar_171 * max ((
          (tmpvar_227 - tmpvar_228)
         / tmpvar_228), 1.0));
        sumS_199 = 0.0;
        cntS_198 = 0;
        for (lowp int xo_197 = -(tmpvar_229); xo_197 <= r_201; xo_197++) {
          for (lowp int yo_230 = -(r_201); yo_230 <= r_201; yo_230++) {
            lowp vec2 tmpvar_231;
            tmpvar_231.x = float(xo_197);
            tmpvar_231.y = float(yo_230);
            lowp vec3 tmpvar_232;
            tmpvar_232.xy = clamp ((pc_191.xy + (tmpvar_231 * texelSize_192)), tileMin_194, tileMax_195);
            tmpvar_232.z = (pc_191.z - dynBias_200);
            sumS_199 = (sumS_199 + texture (shadowMapDetail, tmpvar_232));
            cntS_198++;
          };
        };
        lowp float tmpvar_233;
        lowp float tmpvar_234;
        tmpvar_234 = clamp (clamp ((
          (tmpvar_227 - tmpvar_228)
         / tmpvar_190), 0.0, 1.0), 0.0, 1.0);
        tmpvar_233 = (tmpvar_234 * (tmpvar_234 * (3.0 - 
          (2.0 * tmpvar_234)
        )));
        tmpvar_196 = mix ((sumS_199 / float(cntS_198)), 1.0, tmpvar_233);
      };
      highp vec2 tmpvar_235;
      tmpvar_235.x = (tmpvar_176 * tmpvar_196);
      tmpvar_235.y = mix (((tmpvar_196 * 0.7) + 0.3), 1.0, tmpvar_176);
      shadow_4 = tmpvar_235;
    };
  };
  directionPower_10 = (tmpvar_11 * shadow_4.x);
  indirectPower_9 = (indirectPower_9 * shadow_4.y);
  color_2 = (tmpvar_3.xyz * mix (vec3(indirectPower_9), vec3(1.0, 1.0, 1.0), directionPower_10));
  lowp vec4 tmpvar_236;
  tmpvar_236.xyz = color_2;
  tmpvar_236.w = alpha_1;
  FragColor = tmpvar_236;
}

