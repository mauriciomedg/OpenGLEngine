#version 330 core

in vec2 vUv;
out vec4 FragColor;

uniform sampler2D buf0;

void main()
{
    float d = texture(buf0, vUv).r;
    FragColor = vec4(vec3(d), 1.0);
}
