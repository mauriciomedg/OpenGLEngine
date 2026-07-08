#version 330 core

in vec2 vUv;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vUv, 0.0, 1.0);
}
