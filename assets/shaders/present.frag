#version 330 core

in vec2 vUv;
out vec4 FragColor;

uniform sampler2D buf0;

void main()
{
    FragColor = texture(buf0, vUv);
}
