#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 resolution;

out vec4 finalColor;

void main(void)
{
    float x = 1.f / resolution.x;
    float y = 1.f / resolution.y;

    vec4 horiz_edge = vec4(0.f);
    horiz_edge -= texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)) * 1.f;
    horiz_edge -= texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y))* 2.f;
    horiz_edge -= texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)) * 1.f;
    horiz_edge += texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)) * 1.f;
    horiz_edge += texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y)) * 2.f;
    horiz_edge += texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)) * 1.f;

    vec4 vert_edge = vec4(0.f);
    vert_edge -= texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)) * 1.f;
    vert_edge -= texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y - y)) * 2.f;
    vert_edge -= texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)) * 1.f;
    vert_edge += texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)) * 1.f;
    vert_edge += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y + y)) * 2.f;
    vert_edge += texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)) * 1.f;

    vec3 edge = sqrt((horiz_edge.rgb * horiz_edge.rgb) + (vert_edge.rgb * vert_edge.rgb));
    finalColor = vec4(edge, texture(texture0, fragTexCoord).a);
}
