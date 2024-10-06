#version 330 core

out vec4 FragColor;

in vec3 v_normals;
in vec3 v_fragmentPosition;
in vec4 v_fragmentLightSpacePosition;
in vec2 v_textureCoords;

uniform vec4 baseColor;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform float lightIntensity;

float getShadow()
{
    return 0.0;
}

void main()
{
    // set normals
    vec3 normalDirection = normalize(v_normals);

    // light variables
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 lightDirection = normalize(lightPosition - v_fragmentPosition);
    // float lightIntensity = 0.3;

    // ambient light
    float ambientStrength = 0.025; 
    vec3 ambientColor = vec3(1.0, 1.0, 1.0);
    vec3 ambient = ambientColor * ambientStrength;

    // diffuse light
    float diffuseValue = max(0.0, dot(lightDirection, normalDirection));
    vec3 diffuse = lightColor * diffuseValue * lightIntensity;

    // specular light
    float specularStrength = 0.75;
    float glossiness = 32.0;
    vec3 viewDirection = normalize(cameraPosition - v_fragmentPosition);
    vec3 halfwayDirection = normalize(viewDirection + lightDirection);
    float spec = pow(max(dot(halfwayDirection, normalDirection), 0.0), glossiness);
    vec3 specular = lightColor * spec * specularStrength * lightIntensity;

    // shadow
    float shadow = getShadow();

    // final lighting value
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    // final color = base color * lighting color
    vec4 finalColor = vec4(baseColor.x * lighting.x, baseColor.y * lighting.y, baseColor.z * lighting.z, 1.0);

    FragColor = texture(diffuseTexture, v_textureCoords) * finalColor;
}