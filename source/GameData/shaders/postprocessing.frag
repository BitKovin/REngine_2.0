#version 300 es
precision highp float;

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform ivec2 screenResolution;

// Define a 4x4 Bayer matrix, normalized to [0, 1)
const float bayer4x4[16] = float[16](
    0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
   12.0 / 16.0,  4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0,
    3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0,
   15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0
);

void main() {
    // Calculate the position within the 4x4 Bayer matrix
    int x = int(gl_FragCoord.x) % 4;
    int y = int(gl_FragCoord.y) % 4;
    int index = y * 4 + x;
    float bayer_value = bayer4x4[index];
    
    // Sample the texture color (16-bit precision, normalized to [0,1])
    vec3 color = texture(screenTexture, TexCoords).rgb;
    
    vec3 colorHue = normalize(color);
    float colorBrightness = length(color);
    colorHue = mix(colorHue, vec3(1,0,0),0.2);
    color = colorHue * colorBrightness;

    // Apply dithering by adding the scaled Bayer value to RGB channels
    // Use /256.0 for precise dithering range
    color += bayer_value / 256.0;
    
    // Output the dithered color
    FragColor = vec4(color,1);
}