#version 300 es
layout(location=0) in vec2 a_Position;
layout(location=1) in vec2 a_TexCoord;
uniform mat4 u_Model;
uniform mat4 u_Projection;
out vec2 v_TexCoord;
out vec2 v_LocalUV;
void main ()
{
  v_LocalUV = a_TexCoord;
  v_TexCoord = a_TexCoord;
  vec4 tmpvar_1;
  tmpvar_1.zw = vec2(0.0, 1.0);
  tmpvar_1.xy = a_Position;
  gl_Position = ((u_Projection * u_Model) * tmpvar_1);
}

