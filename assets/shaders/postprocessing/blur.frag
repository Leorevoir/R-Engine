#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main(void)
{
    vec3 tex_color = texture(texture0, fragTexCoord).rgb*weight[0];

    for (int i = 1; i < 3; i++)
    {
        tex_color += texture(texture0, fragTexCoord + vec2(offset[i])/resolution.x, 0.0).rgb*weight[i];
        tex_color += texture(texture0, fragTexCoord - vec2(offset[i])/resolution.x, 0.0).rgb*weight[i];
    }
    finalColor = vec4(tex_color, 1.0);
}
