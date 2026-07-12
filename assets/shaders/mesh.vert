#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec4 aInstanceColumn0;
layout (location = 4) in vec4 aInstanceColumn1;
layout (location = 5) in vec4 aInstanceColumn2;
layout (location = 6) in vec4 aInstanceColumn3;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightViewProjection;
uniform bool uUseInstancing;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec3 vColor;
out vec4 vLightSpacePosition;

void main()
{
    mat4 instanceModel = mat4(
        aInstanceColumn0,
        aInstanceColumn1,
        aInstanceColumn2,
        aInstanceColumn3);
    mat4 model = uUseInstancing ? instanceModel : uModel;

    vec4 worldPosition = model * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = mat3(transpose(inverse(model))) * aNormal;
    vColor = aColor;
    vLightSpacePosition = uLightViewProjection * worldPosition;
    gl_Position = uProjection * uView * worldPosition;
}
