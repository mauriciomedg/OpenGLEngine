#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 3) in vec4 aInstanceColumn0;
layout (location = 4) in vec4 aInstanceColumn1;
layout (location = 5) in vec4 aInstanceColumn2;
layout (location = 6) in vec4 aInstanceColumn3;

uniform mat4 uModel;
uniform mat4 uLightViewProjection;
uniform bool uUseInstancing;

void main()
{
    mat4 instanceModel = mat4(
        aInstanceColumn0,
        aInstanceColumn1,
        aInstanceColumn2,
        aInstanceColumn3);
    mat4 model = uUseInstancing ? instanceModel : uModel;

    gl_Position = uLightViewProjection * model * vec4(aPosition, 1.0);
}
