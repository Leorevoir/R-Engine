#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
out vec4 finalColor;

const mat3 deuteranopia_matrix = mat3
(
    0.625, 0.375, 0.0,
    0.7,   0.3,   0.0,
    0.0,   0.3,   0.7
);

void main(void)
{
    vec3 tex_color = texture(texture0, fragTexCoord).rgb;
    vec3 simulated_color = deuteranopia_matrix * tex_color;

    finalColor = vec4(simulated_color, 1.0);
}
