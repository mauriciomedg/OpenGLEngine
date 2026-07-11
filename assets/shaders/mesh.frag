#version 330 core

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vColor;
in vec4 vLightSpacePosition;

out vec4 FragColor;

uniform sampler2D shadowMap;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform bool uEnablePcf;

float sampleShadow(vec3 projected, float bias)
{
    float closestDepth = texture(shadowMap, projected.xy).r;
    return projected.z - bias > closestDepth ? 1.0 : 0.0;
}

float calculateShadow(vec4 lightSpacePosition, vec3 normal, vec3 lightDir)
{
    vec3 projected = lightSpacePosition.xyz / lightSpacePosition.w;
    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0)
    {
        return 0.0;
    }

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    if (!uEnablePcf)
    {
        return sampleShadow(projected, bias);
    }

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float pcfDepth = texture(shadowMap, projected.xy + vec2(x, y) * texelSize).r;
            shadow += projected.z - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec3 normal = normalize(vWorldNormal);
    vec3 lightDir = normalize(-uLightDirection);
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    float shadow = calculateShadow(vLightSpacePosition, normal, lightDir);

    vec3 ambient = uAmbientColor * vColor;
    vec3 diffuse = uLightColor * diffuseFactor * vColor * (1.0 - shadow);

    FragColor = vec4(ambient + diffuse, 1.0);
}
