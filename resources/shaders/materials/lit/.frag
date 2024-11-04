#version 330 core

out vec4 FragColor;

in vec3 v_normals;
in vec3 v_fragmentPosition;
in vec4 v_fragmentLightSpacePosition;
in vec2 v_textureCoords;

const float gamma = 2.2;
const float PI = 3.14159265359;

vec2 uv;

struct Scene {
    sampler2D shadowMap;
    vec3 cameraPosition;

    vec3 ambientColor;
    float ambientStrength;
};
uniform Scene scene;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform Light light;

struct Material {
    vec4 baseColor;

    vec2 tiling;
    vec2 offset;

    bool enableAlbedoMap;
    sampler2D albedoMap;

    bool enableNormalMap;
    sampler2D normalMap;

    float roughness;
    bool enableRoughnessMap;
    sampler2D roughnessMap;

    float metallic;
    bool enableMetallicMap;
    sampler2D metallicMap;

    bool enableAmbientOcclusionMap;
    sampler2D ambientOcclusionMap;
};
uniform Material material;

float pcf(vec3 projectionCoords, float currentDepth, float bias, float smoothing, int kernelRadius) 
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(scene.shadowMap, 0);
    
    int sampleCount = 0;
    float weightSum = 0.0;

    for (int x = -kernelRadius; x <= kernelRadius; ++x) {
        for (int y = -kernelRadius; y <= kernelRadius; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize * smoothing;
            float pcfDepth = texture(scene.shadowMap, projectionCoords.xy + offset).r;
            float dist = length(vec2(x, y));
            float weight = exp(-dist * dist / (2.0 * float(kernelRadius) * float(kernelRadius)));
            shadow += weight * (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
            weightSum += weight;
            sampleCount++;
        }
    }

    shadow = shadow / weightSum;
    
    return shadow;
}

float getShadow()
{
    vec3 projectionCoordinates = v_fragmentLightSpacePosition.xyz / v_fragmentLightSpacePosition.w;
    projectionCoordinates = projectionCoordinates * 0.5 + 0.5;

    if(projectionCoordinates.z > 1.0) return 0.0f;

    float closestDepth = texture(scene.shadowMap, projectionCoordinates.xy).r;
    float currentDepth = projectionCoordinates.z;

    vec3 normalDirection = normalize(v_normals);
    vec3 lightDirection = normalize(light.position - v_fragmentPosition);

    float maxBias = 0.01;
    float minBias = 0.005;
    float bias = max(maxBias * dot(normalDirection, lightDirection), minBias);
    bias = 0.001f;

    float shadow = pcf(projectionCoordinates, currentDepth, bias, 1.0, 3);

    return shadow;
}

vec3 fresnelSchlick(float VdotN, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotN, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec4 shadePBR() {
    // get albedo value
    vec3 albedo = vec3(1.0);
    if(material.enableAlbedoMap){
        albedo = pow(texture(material.albedoMap, uv).rgb, vec3(gamma));
    }
    albedo *= vec3(material.baseColor);

    // get roughness value
    float roughness = material.roughness;
    if(material.enableRoughnessMap){
        roughness = texture(material.roughnessMap, uv).r;
    }

    // get metallic value
    float metallic = material.metallic;
    if(material.enableMetallicMap){
        metallic = texture(material.metallicMap, uv).r;
    }

    // get ambient occlusion value
    float ambientOcclusion = 1.0;
    if(material.enableAmbientOcclusionMap){
        ambientOcclusion = texture(material.ambientOcclusionMap, uv).r;
    }

    vec3 N = normalize(v_normals); // normal direction
    vec3 V = normalize(scene.cameraPosition - v_fragmentPosition); // view direction

    vec3 F0 = vec3(0.04); // base reflectivity
    F0 = mix(F0, albedo, metallic);

    // calculate each lights impact
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; ++i) 
    {
        vec3 L = normalize(light.position - v_fragmentPosition); // light direction
        vec3 H = normalize(V + L); // halfway direction

        // per-light radiance
        // float distance = length(light.position - v_fragmentPosition);
        // float attenuation = 1.0 / (distance * distance);
        float attenuation = 1.0; // full attenuation for directional lights
        vec3 radiance = light.color * light.intensity * attenuation;
        
        // cook-torrance brdf
        float NDF = distributionGGX(N, H, roughness);        
        float G = geometrySmith(N, V, L, roughness);      
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;  
            
        // add to outgoing radiance
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }

    // shadow value
    float shadow = getShadow();
  
    // ambient component
    vec3 ambient = vec3(scene.ambientStrength) * albedo * ambientOcclusion;
    vec3 color = ambient + Lo * (1.0 - shadow);
	
    // gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / gamma));

    vec4 finalColor = vec4(color, 1.0);
    return finalColor;
}

void main()
{
    uv = v_textureCoords * material.tiling + material.offset;
    FragColor = shadePBR();
}