#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
out vec4 finalColor;

const mat3 protanopia_matrix = mat3
(
    0.56667, 0.43333, 0.0,
    0.55833, 0.44167, 0.0,
    0.0,     0.24167, 0.75833
);

void main()
{
    vec3 tex_color = texture(texture0, fragTexCoord).rgb;
    vec3 simulated_color = protanopia_matrix * tex_color;

    finalColor = vec4(simulated_color, 1.0);
}
