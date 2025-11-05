#version 300 es
precision highp float;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec4 u_Color;
out lowp vec4 FragColor;
void main ()
{
  FragColor = (texture (u_Texture, v_TexCoord) * u_Color);
}

