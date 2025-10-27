#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;

out vec4 finalColor;

const mat3 tritanopia_matrix = mat3
(
    0.95, 0.05,  0.0,
    0.0,  0.433, 0.567,
    0.0,  0.475, 0.525
);

void main()
{
    vec3 texelColor = texture(texture0, fragTexCoord).rgb;
    vec3 simulatedColor = tritanopia_matrix * texelColor;

    finalColor = vec4(simulatedColor, 1.0);
}
