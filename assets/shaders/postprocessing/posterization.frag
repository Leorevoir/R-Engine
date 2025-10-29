#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

const float gamma = 0.6f;
const float numColors = 8.f;

void main(void)
{
    vec3 tex_color = texture(texture0, fragTexCoord.xy).rgb;

    tex_color = pow(tex_color, vec3(gamma, gamma, gamma));
    tex_color = tex_color * numColors;
    tex_color = floor(tex_color);
    tex_color = tex_color / numColors;
    tex_color = pow(tex_color, vec3(1.0 / gamma));
    finalColor = vec4(tex_color, 1.0);
}
