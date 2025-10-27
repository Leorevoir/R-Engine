#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

uniform float time;

void main(void)
{
    float frequency = resolution.y / 3.f;
    float global_pos = (fragTexCoord.y) * frequency;
    float wave_pos = cos((fract(global_pos) - 0.5f) * 3.14f);
    vec4 tex_color = texture(texture0, fragTexCoord);

    finalColor = mix(vec4(0.f, 0.3f, 0.f, 0.f), tex_color, wave_pos);
}
