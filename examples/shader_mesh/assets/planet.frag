#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec4 customColor; 
uniform float globalTime;

void main()
{
    float pulsatingRed = 0.5 + 0.5 * sin(globalTime * 3.0);
    
    vec3 final_rgb = vec3(pulsatingRed, customColor.g, customColor.b);

    finalColor = vec4(final_rgb, 1.0);
}
