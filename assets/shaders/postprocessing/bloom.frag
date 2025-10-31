#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

const float samples = 5.0;
const float quality = 2.5;
const int range = 2;

void main(void)
{
    vec4 sum = vec4(0);
    vec2 size_factor = vec2(1) / resolution * quality;
    vec4 source = texture(texture0, fragTexCoord);

    for (int x = -range; x <= range; ++x)
    {
        for (int y = -range; y <= range; ++y)
        {
            sum += texture(texture0, fragTexCoord + vec2(x, y) * size_factor);
        }
    }
    finalColor = ((sum / (samples * samples)) + source) * colDiffuse;
}
