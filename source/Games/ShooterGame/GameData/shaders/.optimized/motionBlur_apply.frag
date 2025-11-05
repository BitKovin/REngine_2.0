#version 300 es
precision highp float;
out lowp vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D blurTexture;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1 = texture (blurTexture, TexCoords);
  lowp vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.xyz = ((tmpvar_1.xyz * tmpvar_1.w) + (texture (screenTexture, TexCoords).xyz * (1.0 - tmpvar_1.w)));
  FragColor = tmpvar_2;
}

