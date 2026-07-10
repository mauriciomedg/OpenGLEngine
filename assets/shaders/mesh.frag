#version 330 core

in vec3 vColor;
out vec4 FragColor;

uniform sampler2D shadowMap;

void main()
{
    float shadowPlaceholder = texture(shadowMap, vec2(0.5)).r * 0.0;
    FragColor = vec4(vColor + vec3(shadowPlaceholder), 1.0);
}
