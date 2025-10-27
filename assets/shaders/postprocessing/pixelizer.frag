#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

uniform float pixelWidth = 5.f;
uniform float pixelHeight = 5.f;

void main(void)
{
    float dx = pixelWidth * (1.f / resolution.x);
    float dy = pixelHeight*(1.f / resolution.y);
    vec2 coord = vec2(dx * floor(fragTexCoord.x / dx), dy * floor(fragTexCoord.y / dy));
    vec3 tc = texture(texture0, coord).rgb;

    finalColor = vec4(tc, 1.0);
}
