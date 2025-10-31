#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main(void)
{
    vec4 tex_color = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    float gray = dot(tex_color.rgb, vec3(0.299, 0.587, 0.114));

    finalColor = vec4(gray, gray, gray, tex_color.a);
}
