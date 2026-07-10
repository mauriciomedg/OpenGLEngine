#version 330 core

in vec2 vUv;
out float FragColor;

uniform mat4 uInverseModel;
uniform float uSliceZ;

void main()
{
    vec2 slicePosition = vUv * 2.0 - 1.0;
    vec4 localPosition = uInverseModel * vec4(slicePosition, uSliceZ, 1.0);
    vec3 p = localPosition.xyz;

    vec3 distanceToFace = vec3(0.5) - abs(p);
    float inside = min(min(distanceToFace.x, distanceToFace.y), distanceToFace.z);
    float density = smoothstep(-0.015, 0.015, inside);

    FragColor = density;
}
