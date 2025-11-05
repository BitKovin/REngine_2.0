#version 300 es
precision highp float;
in vec4 g_world;
uniform vec3 cameraPosition;
uniform lowp samplerCube s_bspTexture;
out lowp vec4 FragColor;
void main ()
{
  FragColor = texture (s_bspTexture, (normalize((g_world.xyz - cameraPosition)) * vec3(-1.0, 1.0, 1.0)));
}

