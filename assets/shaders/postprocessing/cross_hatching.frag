#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

const float hatch_offset_y = 5.f;
const float lum_threshold_01 = 0.9f;
const float lum_threshold_02 = 0.7f;
const float lum_threshold_03 = 0.5f;
const float lum_threshold_04 = 0.3f;

void main(void)
{
    vec3 tc = vec3(1.f, 1.f, 1.f);
    float lum = length(texture(texture0, fragTexCoord).rgb);

    if (lum < lum_threshold_01)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.f) == 0.f)
        {
            tc = vec3(0.f, 0.f, 0.f);
        }
    }
    if (lum < lum_threshold_02)
    {
        if (mod(gl_FragCoord.x - gl_FragCoord.y, 10.f) == 0.f)
        {
            tc = vec3(0.f, 0.f, 0.f);
        }
    }
    if (lum < lum_threshold_03)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y - hatch_offset_y, 10.f) == 0.f)
        {
            tc = vec3(0.f, 0.f, 0.f);
        }
    }
    if (lum < lum_threshold_04)
    {
        if (mod(gl_FragCoord.x - gl_FragCoord.y - hatch_offset_y, 10.f) == 0.f)
        {
            tc = vec3(0.f, 0.f, 0.f);
        }
    }
    finalColor = vec4(tc, 1.f);
}
