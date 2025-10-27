#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float PI = 3.1415926535;
const float PI_180 = 0.017453292519444;
const float APERTURE = 178.f;
const float APERTURE_HALF = 0.5f * APERTURE * (PI_180);

void main(void)
{
    float max_factor = sin(APERTURE_HALF);

    vec2 uv = vec2(0);
    vec2 xy = 2.0 * fragTexCoord.xy - 1.0;
    float d = length(xy);

    if (d < (2.0 - max_factor))
    {
        d = length(xy * max_factor);
        float z = sqrt(1.0 - d * d);
        float r = atan(d, z) / PI;
        float phi = atan(xy.y, xy.x);

        uv.x = r * cos(phi) + 0.5;
        uv.y = r * sin(phi) + 0.5;
    }
    else
    {
        uv = fragTexCoord.xy;
    }

    fragColor = texture(texture0, uv);
}
