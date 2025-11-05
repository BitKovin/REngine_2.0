#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
out highp vec4 FragColor;
uniform sampler2D u_texture;
uniform sampler2D depthTexture;
uniform vec2 u_invViewport;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1 = (texture (u_texture, v_texcoord) * v_color);
  if ((tmpvar_1.w < 0.001)) {
    discard;
  };
  highp vec2 tmpvar_2;
  tmpvar_2 = (gl_FragCoord.xy * u_invViewport);
  lowp vec4 tmpvar_3;
  tmpvar_3 = texture (depthTexture, tmpvar_2);
  highp vec4 tmpvar_4;
  tmpvar_4.xyz = tmpvar_1.xyz;
  tmpvar_4.w = (tmpvar_1.w * clamp ((
    (gl_FragCoord.z - tmpvar_3.x)
   / 0.04), 0.0, 1.0));
  FragColor = tmpvar_4;
}

