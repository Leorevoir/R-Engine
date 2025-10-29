#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

const float stitching_size = 6.f;
uniform int invert = 0;

vec4 _post_fx(sampler2D tex, vec2 uv)
{
    vec4 c = vec4(0.f);
    float size = stitching_size;
    vec2 c_pos = uv * resolution;
    vec2 tl_pos = floor(c_pos / vec2(size, size));

    tl_pos *= size;

    int rem_x = int(mod(c_pos.x, size));
    int rem_y = int(mod(c_pos.y, size));

    if (rem_x == 0 && rem_y == 0)
    {
        tl_pos = c_pos;
    }

    vec2 bl_pos = tl_pos;

    bl_pos.y += (size - 1.0);
    if ((rem_x == rem_y) || (((int(c_pos.x) - int(bl_pos.x)) == (int(bl_pos.y) - int(c_pos.y)))))
    {
        if (invert == 1) 
        {
            c = vec4(0.2f, 0.15f, 0.05f, 1.f);
        }
        else 
        {
            c = texture(tex, tl_pos / resolution) * 1.4f;
        }
    }
    else
    {
        if (invert == 1)
        {
            c = texture(tex, tl_pos / resolution) * 1.4f;
        }
        else
        {
            c = vec4(0.f, 0.f, 0.f, 1.f);
        }
    }
    return c;
}

void main(void)
{
    vec3 tc = _post_fx(texture0, fragTexCoord).rgb;

    finalColor = vec4(tc, 1.f);
}
