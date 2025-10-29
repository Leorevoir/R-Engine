#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;

out vec4 finalColor;

uniform float contrast;

void main(void)
{
    vec4 tex_color = texture(texture0, fragTexCoord);
    vec3 contrast_color = (tex_color.rgb - 0.5) * contrast + 0.5;

    finalColor = vec4(clamp(contrast_color, 0.0, 1.0), tex_color.a);
}
