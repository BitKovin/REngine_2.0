#version 300 es
precision highp float;
in vec2 v_LocalUV;
out lowp vec4 FragColor;
uniform sampler2D u_Texture;
uniform vec4 u_Color;
void main ()
{
  lowp int regionSizeY_1;
  lowp int regionStartY_2;
  lowp float tex_f_y_3;
  lowp int regionSizeX_4;
  lowp int regionStartX_5;
  lowp float tex_f_x_6;
  lowp float sy_7;
  lowp float sx_8;
  lowp ivec2 tmpvar_9;
  tmpvar_9 = textureSize (u_Texture, 0);
  lowp int tmpvar_10;
  tmpvar_10 = clamp (16, 0, tmpvar_9.x);
  lowp int tmpvar_11;
  tmpvar_11 = clamp (16, 0, (tmpvar_9.x - tmpvar_10));
  lowp int tmpvar_12;
  tmpvar_12 = clamp (16, 0, tmpvar_9.y);
  lowp int tmpvar_13;
  tmpvar_13 = clamp (16, 0, (tmpvar_9.y - tmpvar_12));
  lowp int tmpvar_14;
  tmpvar_14 = max (1, ((tmpvar_9.x - tmpvar_10) - tmpvar_11));
  lowp int tmpvar_15;
  tmpvar_15 = max (1, ((tmpvar_9.y - tmpvar_12) - tmpvar_13));
  vec2 tmpvar_16;
  tmpvar_16 = (1.0/(max (max (
    abs(dFdx(v_LocalUV))
  , 
    abs(dFdy(v_LocalUV))
  ), vec2(1e-6, 1e-6))));
  float tmpvar_17;
  tmpvar_17 = max (1.0, tmpvar_16.x);
  float tmpvar_18;
  tmpvar_18 = max (1.0, tmpvar_16.y);
  lowp float tmpvar_19;
  tmpvar_19 = float((tmpvar_10 + tmpvar_11));
  lowp float tmpvar_20;
  tmpvar_20 = float((tmpvar_12 + tmpvar_13));
  sx_8 = 1.0;
  if (((tmpvar_19 > 1e-6) && (tmpvar_17 < tmpvar_19))) {
    sx_8 = (tmpvar_17 / tmpvar_19);
  };
  sy_7 = 1.0;
  if (((tmpvar_20 > 1e-6) && (tmpvar_18 < tmpvar_20))) {
    sy_7 = (tmpvar_18 / tmpvar_20);
  };
  lowp float tmpvar_21;
  tmpvar_21 = (float(tmpvar_10) * sx_8);
  lowp float tmpvar_22;
  tmpvar_22 = (float(tmpvar_11) * sx_8);
  lowp float tmpvar_23;
  tmpvar_23 = (float(tmpvar_12) * sy_7);
  lowp float tmpvar_24;
  tmpvar_24 = (float(tmpvar_13) * sy_7);
  lowp float tmpvar_25;
  tmpvar_25 = (tmpvar_21 / tmpvar_17);
  lowp float tmpvar_26;
  tmpvar_26 = (tmpvar_22 / tmpvar_17);
  lowp float tmpvar_27;
  tmpvar_27 = (tmpvar_24 / tmpvar_18);
  lowp float tmpvar_28;
  tmpvar_28 = (tmpvar_23 / tmpvar_18);
  lowp float tmpvar_29;
  tmpvar_29 = (max (1.0, (
    (tmpvar_17 - tmpvar_21)
   - tmpvar_22)) / float(tmpvar_14));
  lowp float tmpvar_30;
  tmpvar_30 = (max (1.0, (
    (tmpvar_18 - tmpvar_23)
   - tmpvar_24)) / float(tmpvar_15));
  bool tmpvar_31;
  tmpvar_31 = (tmpvar_29 > 1.0);
  bool tmpvar_32;
  tmpvar_32 = (tmpvar_30 > 1.0);
  if ((v_LocalUV.x < tmpvar_25)) {
    regionStartX_5 = 0;
    regionSizeX_4 = tmpvar_10;
    tex_f_x_6 = (((v_LocalUV.x / 
      max (tmpvar_25, 1e-6)
    ) * float(tmpvar_10)) - 0.5);
  } else {
    if ((v_LocalUV.x > (1.0 - tmpvar_26))) {
      lowp float tmpvar_33;
      tmpvar_33 = ((v_LocalUV.x - (1.0 - tmpvar_26)) / max (tmpvar_26, 1e-6));
      regionStartX_5 = (tmpvar_10 + tmpvar_14);
      lowp int tmpvar_34;
      if ((tmpvar_11 > 0)) {
        tmpvar_34 = tmpvar_11;
      } else {
        tmpvar_34 = 1;
      };
      regionSizeX_4 = tmpvar_34;
      tex_f_x_6 = (((tmpvar_33 * 
        float(tmpvar_34)
      ) - 0.5) + float(regionStartX_5));
    } else {
      lowp float t_35;
      lowp float tmpvar_36;
      tmpvar_36 = ((v_LocalUV.x - tmpvar_25) / max (1e-6, (
        (1.0 - tmpvar_25)
       - tmpvar_26)));
      t_35 = tmpvar_36;
      if (tmpvar_31) {
        t_35 = fract((tmpvar_36 * tmpvar_29));
      };
      regionStartX_5 = tmpvar_10;
      regionSizeX_4 = tmpvar_14;
      tex_f_x_6 = (((t_35 * 
        float(tmpvar_14)
      ) - 0.5) + float(tmpvar_10));
    };
  };
  if ((v_LocalUV.y < tmpvar_27)) {
    lowp float tmpvar_37;
    tmpvar_37 = (v_LocalUV.y / max (tmpvar_27, 1e-6));
    regionStartY_2 = 0;
    lowp int tmpvar_38;
    if ((tmpvar_13 > 0)) {
      tmpvar_38 = tmpvar_13;
    } else {
      tmpvar_38 = 1;
    };
    regionSizeY_1 = tmpvar_38;
    tex_f_y_3 = ((tmpvar_37 * float(tmpvar_38)) - 0.5);
  } else {
    if ((v_LocalUV.y > (1.0 - tmpvar_28))) {
      lowp float tmpvar_39;
      tmpvar_39 = ((v_LocalUV.y - (1.0 - tmpvar_28)) / max (tmpvar_28, 1e-6));
      regionStartY_2 = (tmpvar_13 + tmpvar_15);
      lowp int tmpvar_40;
      if ((tmpvar_12 > 0)) {
        tmpvar_40 = tmpvar_12;
      } else {
        tmpvar_40 = 1;
      };
      regionSizeY_1 = tmpvar_40;
      tex_f_y_3 = (((tmpvar_39 * 
        float(tmpvar_40)
      ) - 0.5) + float(regionStartY_2));
    } else {
      lowp float t_41;
      lowp float tmpvar_42;
      tmpvar_42 = ((v_LocalUV.y - tmpvar_27) / max (1e-6, (
        (1.0 - tmpvar_27)
       - tmpvar_28)));
      t_41 = tmpvar_42;
      if (tmpvar_32) {
        t_41 = fract((tmpvar_42 * tmpvar_30));
      };
      regionStartY_2 = tmpvar_13;
      regionSizeY_1 = tmpvar_15;
      tex_f_y_3 = (((t_41 * 
        float(tmpvar_15)
      ) - 0.5) + float(tmpvar_13));
    };
  };
  lowp float fy_43;
  lowp float fx_44;
  lowp int iy1_45;
  lowp int ix1_46;
  lowp int tmpvar_47;
  tmpvar_47 = ((regionStartX_5 + regionSizeX_4) - 1);
  lowp int tmpvar_48;
  tmpvar_48 = ((regionStartY_2 + regionSizeY_1) - 1);
  lowp int tmpvar_49;
  tmpvar_49 = int(floor(tex_f_x_6));
  lowp int tmpvar_50;
  tmpvar_50 = int(floor(tex_f_y_3));
  lowp int tmpvar_51;
  tmpvar_51 = clamp (tmpvar_49, regionStartX_5, tmpvar_47);
  lowp int tmpvar_52;
  tmpvar_52 = clamp (tmpvar_50, regionStartY_2, tmpvar_48);
  lowp int tmpvar_53;
  tmpvar_53 = (tmpvar_51 + 1);
  ix1_46 = tmpvar_53;
  if ((tmpvar_53 > tmpvar_47)) {
    ix1_46 = tmpvar_47;
  };
  lowp int tmpvar_54;
  tmpvar_54 = (tmpvar_52 + 1);
  iy1_45 = tmpvar_54;
  if ((tmpvar_54 > tmpvar_48)) {
    iy1_45 = tmpvar_48;
  };
  fx_44 = (tex_f_x_6 - float(tmpvar_49));
  if (((regionSizeX_4 <= 1) || (tmpvar_51 == ix1_46))) {
    fx_44 = 0.0;
  };
  fy_43 = (tex_f_y_3 - float(tmpvar_50));
  if (((regionSizeY_1 <= 1) || (tmpvar_52 == iy1_45))) {
    fy_43 = 0.0;
  };
  lowp ivec2 tmpvar_55;
  tmpvar_55.x = tmpvar_51;
  tmpvar_55.y = tmpvar_52;
  lowp ivec2 tmpvar_56;
  tmpvar_56.x = ix1_46;
  tmpvar_56.y = tmpvar_52;
  lowp ivec2 tmpvar_57;
  tmpvar_57.x = tmpvar_51;
  tmpvar_57.y = iy1_45;
  lowp ivec2 tmpvar_58;
  tmpvar_58.x = ix1_46;
  tmpvar_58.y = iy1_45;
  FragColor = (mix (mix (texelFetch (u_Texture, tmpvar_55, 0), texelFetch (u_Texture, tmpvar_56, 0), fx_44), mix (texelFetch (u_Texture, tmpvar_57, 0), texelFetch (u_Texture, tmpvar_58, 0), fx_44), fy_43) * u_Color);
}

