#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main(void)
{
    vec3 tex_color = texture(texture0, fragTexCoord).rgb;
    vec3 colors[3];

    colors[0] = vec3(0.f, 0.f, 1.f);
    colors[1] = vec3(1.f, 1.f, 0.f);
    colors[2] = vec3(1.f, 0.f, 0.f);

    float lum = (tex_color.r + tex_color.g + tex_color.b) / 3.f;

    int ix = (lum < 0.5f) ? 0 : 1;

    vec3 tc = mix(colors[ix], colors[ix + 1], (lum - float(ix) * 0.5f) / 0.5f);

    finalColor = vec4(tc, 1.f);
}
