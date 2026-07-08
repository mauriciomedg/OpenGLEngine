#version 330 core

in vec2 vUv;

out vec4 FragColor;

void main()
{
    vec3 deepBlue = vec3(0.02, 0.06, 0.09);
    vec3 cyan = vec3(0.08, 0.65, 0.82);
    vec3 color = mix(deepBlue, cyan, smoothstep(0.0, 1.0, vUv.y));
    FragColor = vec4(color, 1.0);
}
