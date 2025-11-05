#version 300 es
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoords;
out vec2 TexCoords;
void main ()
{
  TexCoords = aTexCoords;
  highp vec4 tmpvar_1;
  tmpvar_1.zw = vec2(0.0, 1.0);
  tmpvar_1.xy = aPos;
  gl_Position = tmpvar_1;
}

