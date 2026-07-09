#version 330 core

in vec2 vUv;
out vec4 FragColor;

uniform sampler2D buf0;     // DENSITY
uniform sampler2D buf1;     // NOISE
uniform sampler2D buf2;     // METAL
uniform sampler2D buf3;     // LUNGS_IN
uniform sampler2D echoMask; // ECHOMASK

void main()
{
    float density = texture(buf0, vUv).r;
    float noiseValue = texture(buf1, vUv).r;
    float metal = texture(buf2, vUv).r;
    float lungs = texture(buf3, vUv).r;
    float mask = texture(echoMask, vUv).r;

    float value = density * lungs;
    value += noiseValue * 0.15;
    value += metal * 0.8;
    value *= mask;

    FragColor = vec4(value, value, value, 1.0);
}