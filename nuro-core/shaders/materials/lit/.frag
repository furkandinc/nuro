#version 330 core

#define PI 3.14159265359

#define NO_FOG 0
#define LINEAR_FOG 1
#define EXPONENTIAL_FOG 2
#define EXPONENTIAL_SQUARED_FOG 3

#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 15
#define MAX_SPOT_LIGHTS 8

out vec4 FragColor;

in vec3 v_normal;
in vec2 v_uv;
in mat3 v_tbn;
in mat3 v_tbnTransposed;
in vec3 v_fragmentWorldPosition;
in vec4 v_fragmentLightSpacePosition;

vec2 viewportUv;
vec2 uv;
vec3 normal;

struct Configuration {
    // General parameters
    float gamma;
    bool solidMode;
    vec2 viewportResolution;

    // Shadow parameters
    bool castShadows;
    sampler2D shadowMap;
    float shadowMapResolutionWidth;
    float shadowMapResolutionHeight;

    sampler3D shadowDisk;
    float shadowDiskWindowSize;
    float shadowDiskFilterSize;
    float shadowDiskRadius;

    // World parameters
    vec3 cameraPosition;

    // Lighting parameters
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;

    // SSAO
    bool enableSSAO;
    sampler2D ssaoBuffer;
};
uniform Configuration configuration;

struct DirectionalLight {
    float intensity;
    vec3 direction;
    vec3 color;
    vec3 position; // boilerplate for directional shadows
};
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    float falloff;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];

struct Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
    float falloff;
    float innerCos;
    float outerCos;
};
uniform Spotlight spotlights[MAX_SPOT_LIGHTS];

struct Fog {
    int type;
    vec3 color;
    float data[2];
};
uniform Fog fog;

struct Material {
    vec4 baseColor;

    vec2 tiling;
    vec2 offset;

    bool enableAlbedoMap;
    sampler2D albedoMap;

    float roughness;
    bool enableRoughnessMap;
    sampler2D roughnessMap;

    float metallic;
    bool enableMetallicMap;
    sampler2D metallicMap;

    bool enableNormalMap;
    sampler2D normalMap;
    float normalMapIntensity;

    bool enableOcclusionMap;
    sampler2D occlusionMap;

    bool emission;
    float emissionIntensity;
    vec3 emissionColor;
    bool enableEmissiveMap;
    sampler2D emissiveMap;

    bool enableHeightMap;
    sampler2D heightMap;
    float heightMapScale;
};
uniform Material material;

//
// HELPERS
//

float sqr(float x)
{
    return x * x;
}

//
// SHADOWING
//

// get fragment position as shadow coordinates
vec3 getShadowCoords() {
    vec3 projectionCoords = v_fragmentLightSpacePosition.xyz / v_fragmentLightSpacePosition.w;
    vec3 shadowCoords = projectionCoords * 0.5 + vec3(0.5);
    return shadowCoords;
}

// get bias for directional light
float getShadowBias(vec3 lightDirection) {
    // float diffuseFactor = dot(normal, -lightDirection);
    // float bias = mix(0.0001, 0.0, diffuseFactor);
    float bias = 0.00005;
    return bias;
}

// get hard shadow casted by directional light
float getShadowHard(vec3 lightDirection)
{
    // make sure shadows are enabled
    if (!configuration.castShadows) return 0.0;

    // get shadow coordinates
    vec3 shadowCoords = getShadowCoords();

    // if shadow coordinate's depth is beyond 1.0, fragment isn't in shadow
    if (shadowCoords.z > 1.0) return 0.0;

    // check if fragment is in shadow
    float depth = texture(configuration.shadowMap, shadowCoords.xy).r;
    float bias = getShadowBias(lightDirection);
    float shadow = shadowCoords.z - bias > depth ? 1.0 : 0.0;

    // return shadow value
    return shadow;
}

// get soft shadow casted by directional light
float getShadowSoft(vec3 lightDirection)
{
    // make sure shadows are enabled
    if (!configuration.castShadows) return 0.0;

    // get shadow coordinates
    vec3 shadowCoords = getShadowCoords();

    // if shadow coordinate's depth is beyond 1.0, fragment isn't in shadow
    if (shadowCoords.z > 1.0) return 0.0;

    // initialize offset for sampling shadow map at different positions
    ivec3 offsetCoord;

    // get fractional part of fragment's screen position (for sampling)
    vec2 f = mod(gl_FragCoord.xy, vec2(configuration.shadowDiskWindowSize));

    // assign fractional part to y and z components of offset
    offsetCoord.yz = ivec2(f);

    // create a vec4 for shadow coordinates with a z of 1.0 for sampling
    vec4 sc = vec4(shadowCoords, 1.0);

    // initialize sum for accumulated shadow result
    float sum = 0.0;

    // calculate number of samples to take based on filter size
    int samplesDiv2 = int(configuration.shadowDiskFilterSize * configuration.shadowDiskFilterSize / 2.0);

    // calculate texel size for shadow map based on its dimensions
    float texelWidth = 1.0 / configuration.shadowMapResolutionWidth;
    float texelHeight = 1.0 / configuration.shadowMapResolutionHeight;

    // store texel size in a vec2
    vec2 texelSize = vec2(texelWidth, texelHeight);

    // calculate a small bias to prevent self-shadowing artifacts
    float bias = getShadowBias(lightDirection);
    float depth = 0.0;

    // loop through a small number of samples in 2x2 pattern (8 total)
    for (int i = 0; i < 4; i++) {
        offsetCoord.x = i; // set x offset for this sample

        // fetch offsets from shadow disk texture, scaled by shadow radius
        vec4 Offsets = texelFetch(configuration.shadowDisk, offsetCoord, 0) * configuration.shadowDiskRadius;

        // sample shadow map at first offset location
        sc.xy = shadowCoords.xy + Offsets.rg * texelSize;
        depth = texture(configuration.shadowMap, sc.xy).x;

        // compare depth to shadow coordinate z value to determine if in shadow
        shadowCoords.z - bias > depth ? sum += 1.0 : sum += 0.0;

        // sample shadow map at second offset location
        sc.xy = shadowCoords.xy + Offsets.ba * texelSize;
        depth = texture(configuration.shadowMap, sc.xy).x;

        // compare depth again
        shadowCoords.z - bias > depth ? sum += 1.0 : sum += 0.0;
    }

    // compute shadow result as average of 8 samples
    float shadow = sum / 8.0;

    // if shadow is not fully black or fully white, we need more samples for softening
    if (shadow != 0.0 && shadow != 1.0) {
        // loop through additional samples (beyond initial 8)
        for (int i = 4; i < samplesDiv2; i++) {
            offsetCoord.x = i;

            // fetch more offsets from shadow disk texture
            vec4 Offsets = texelFetch(configuration.shadowDisk, offsetCoord, 0) * configuration.shadowDiskRadius;

            // sample shadow map at first offset location
            sc.xy = shadowCoords.xy + Offsets.rg * texelSize;
            depth = texture(configuration.shadowMap, sc.xy).x;

            // compare depth to shadow coordinate z value
            shadowCoords.z - bias > depth ? sum += 1.0 : sum += 0.0;

            // sample at second offset location
            sc.xy = shadowCoords.xy + Offsets.ba * texelSize;
            depth = texture(configuration.shadowMap, sc.xy).x;

            // compare depth again
            shadowCoords.z - bias > depth ? sum += 1.0 : sum += 0.0;
        }

        // calculate final shadow result as average of all samples
        shadow = sum / float(samplesDiv2 * 2.0);
    }

    // return shadow value
    return shadow;
}

//
// FOG
//

// get linear fog factor
float getLinearFog(float start, float end) {
    float depth = length(v_fragmentWorldPosition - configuration.cameraPosition);
    float fogRange = end - start;
    float fogDistance = end - depth;
    float factor = clamp(fogDistance / fogRange, 0.0, 1.0);
    return factor;
}

// get exponential fog factor
float getExponentialFog(float density) {
    float depth = length(v_fragmentWorldPosition - configuration.cameraPosition);
    float factor = 1 / exp(depth * density);
    return factor;
}

// get exponential squared fog factor
float getExponentialSquaredFog(float density) {
    float depth = length(v_fragmentWorldPosition - configuration.cameraPosition);
    float factor = 1 / exp(sqr(depth * density));
    return factor;
}

//
// PBR FUNCTIONS
//

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
    float numerator = a2;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    return numerator / denominator;
}
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;
    return numerator / denominator;
}
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// evaluate light source contribution 
vec3 evaluateLightSource(
    vec3 V,
    vec3 N,
    vec3 F0,
    float roughness,
    float metallic,
    vec3 albedo,
    float attenuation,
    vec3 L,
    vec3 color,
    float intensity,
    float shadow) {

    // fragment is fully in light sources shadow, no light source contribution
    if (shadow == 1.0) {
        return vec3(0.0);
    }

    // halfway direction
    vec3 H = normalize(V + L);

    // cosine of angle between N and L, indicates effective light contribution from source
    float NdotL = max(dot(N, L), 0.0);

    // no light contribution from source, no light source contribution
    if (NdotL == 0.0)
    {
        return vec3(0.0);
    }

    // no light contribution from source, no light source contribution
    if (max(attenuation, 0.0) == 0.0)
    {
        return vec3(0.0);
    }

    // per-light radiance
    vec3 radiance = color * intensity * attenuation;

    // specular component
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float epsilon = 0.0001;
    vec3 specular = numerator / max(denominator, epsilon);

    // diffuse component (lambertian)
    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;
    vec3 diffuse = kD * albedo / PI;

    // return light source contribution
    vec3 contribution = (diffuse + specular) * radiance * NdotL;
    contribution *= (1.0 - shadow);
    return contribution;
}

//
// PARALLAX OCCLUSION MAPPING
//

// parallax occlusion mapping of given texture coordinates
vec2 POM_getUv(vec2 uvInput) {
    // calculate view direction in tangent space
    vec3 tangentCameraPosition = v_tbnTransposed * configuration.cameraPosition;
    vec3 tangentFragmentPosition = v_tbnTransposed * v_fragmentWorldPosition;
    vec3 V = normalize(tangentCameraPosition - tangentFragmentPosition);

    // setup layers
    const int quality = 8;
    const float minLayers = 32.0;
    const float maxLayers = 64.0;
    float nLayers = mix(maxLayers * quality, minLayers * quality, abs(dot(vec3(0.0, 0.0, 1.0), V)));

    // calculate layer depth
    float layerDepth = 1.0 / nLayers;

    // calculate p and uv delta
    vec2 P = V.xy / V.z * material.heightMapScale;
    vec2 uvDelta = P / nLayers;

    // initialize current uv with input uv
    vec2 uvCurrent = uvInput;

    // sample depth at current uv
    float depthSample = 1.0 - texture(material.heightMap, uvCurrent).r;

    // march along view direction, starting from the beginning
    // loop until current layer depth exceeds or equals sampled depth
    // determines approximate surface intersection point where ray intersects height field
    float currentLayerDepth = 0.0;
    while (currentLayerDepth < depthSample)
    {
        // move uv to next layer
        uvCurrent -= uvDelta;

        // resample depth at new uv current
        depthSample = 1.0 - texture(material.heightMap, uvCurrent).r;

        // add depth to current layer
        currentLayerDepth += layerDepth;
    }

    // calculate occlusion
    vec2 uvPrevious = uvCurrent + uvDelta;
    float depthAfter = depthSample - currentLayerDepth;
    float depthBefore = 1.0 - texture(material.heightMap, uvPrevious).r - currentLayerDepth + layerDepth;
    float weight = depthAfter / (depthAfter - depthBefore);

    // calculate final uv output
    uvCurrent = uvPrevious * weight + uvCurrent * (1.0 - weight);

    // discard if uv output isnt valid
    if (uvCurrent.x > 1.0 || uvCurrent.y > 1.0 || uvCurrent.x < 0.0 || uvCurrent.y < 0.0) {
        discard;
    }

    // return current uv as output
    return uvCurrent;
}

// sample shadow value of given light direction according to parallax occlusion mapped surface with given offset
float POM_sampleShadow(vec3 tangentLightDirection, vec2 uvOffset)
{
    // tangent light direction facing away from surface, early out
    if (tangentLightDirection.z >= 0.0){
        return 0.0;
    }

    // setup layers
    const int quality = 1;
    const float minLayers = 32.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers * quality, minLayers * quality, abs(dot(vec3(0.0, 0.0, 1.0), tangentLightDirection)));

    // initialize current uv with uv mapped by parallax occlusion mapping before and optional offset
    // note: no offset for single sampled hard shadows; needed when multisampling for softshadows
    vec2 uvCurrent = uv + uvOffset;

    // sample depth at current uv
    float depthSample = 1.0 - texture(material.heightMap, uvCurrent).r;
    
    // calculate layer depth
    float layerDepth = 1.0 / numLayers;

    // calculate p and uv delta
    vec2 P = tangentLightDirection.xy / tangentLightDirection.z * material.heightMapScale;
    vec2 uvDelta = P / numLayers;

    // march along light direction in reverse, starting from parallax occlusion mapped depth of pixel
    // loop until:
    // case a) current layer depth exceeds sampled depth (intersection with height field, fragment is in shadow)
    // case b) current layer depth falls below 0 (no intersection with height field, fragment is not in shadow)
    float currentLayerDepth = depthSample;
    while (currentLayerDepth <= depthSample && currentLayerDepth > 0.0)
    {
        uvCurrent += uvDelta;
        depthSample = 1.0 - texture(material.heightMap, uvCurrent).r;
        currentLayerDepth -= layerDepth;
    }

    // determine shadow value, set shaodw if current layer depth exceeded sampled depth (see above)
    float shadow = currentLayerDepth > depthSample ? 1.0 : 0.0;
    
    // return shadow value
    return shadow;
}

// multisample shadow value of given light direction according to parallax occlusion mapped surface and average samples together
float POM_multisampleShadowAverage(vec3 tangentLightDirection)
{
    // get texel size
    vec2 texelSize = 1.0 / textureSize(material.heightMap, 0);

    // calculate square kernel
    int sampleCount = 9;
    int kernelSize = int(sqrt(float(sampleCount)));
    float halfKernel = float(kernelSize) * 0.5;

    // create offsets dynamically based on sample count
    float offsetStep = 2.0 / float(kernelSize); // spacing between offsets
    int samples = 0;
    float shadow = 0.0;

    for (int y = 0; y < kernelSize; y++) {
        for (int x = 0; x < kernelSize; x++) {
            // Compute the offset for each sample
            vec2 offset = vec2(float(x) - halfKernel, float(y) - halfKernel) * texelSize;

            // Sample parallax occlusion shadow with offset
            shadow += POM_sampleShadow(tangentLightDirection, offset);

            // Count the sample
            samples++;
        }
    }

    // Average the shadow values
    return shadow / float(samples);
}

// get hard shadow value for specific light from parallax occlusion mapped height differences
float POM_getShadowHard(vec3 lightPosition){
    // calculate light direction in tangent space
    vec3 tangentLightDirection = normalize(v_tbnTransposed * v_fragmentWorldPosition - v_tbnTransposed * lightPosition);

    // sample shadow once without offset for tangent light direction
    return POM_sampleShadow(tangentLightDirection, vec2(0.0));
}

// get soft shadow value for specific light from parallax occlusion mapped height differences
float POM_getShadowSoft(vec3 lightPosition){
    // calculate light direction in tangent space
    vec3 tangentLightDirection = normalize(v_tbnTransposed * v_fragmentWorldPosition - v_tbnTransposed * lightPosition);

    // multisample shadow for tangent light direction
    return POM_multisampleShadowAverage(tangentLightDirection);
}

//
// TEXTURE SAMPLING
//

// get processed texture coordinates
vec2 getUv() {
    // calculate scaled texture coordinates by material properties
    vec2 _uv = v_uv * material.tiling + material.offset;

    // height map enabled, transform texture coordinates by heightmap
    if (material.enableHeightMap) {
        _uv = POM_getUv(_uv);
    }

    // return texture coordinates
    return _uv;
}

// get normal vector
vec3 getNormal() {
    // no normal mapping, return input normal
    if (!material.enableNormalMap) {
        return v_normal;
    }

    // normal mapping enabled

    // sample normal map
    vec3 N = texture(material.normalMap, uv).rgb;

    // normalize sampled normal
    N = N * 2.0 - vec3(1.0);

    // scale normal x and y by normal map intensity
    N.xy *= material.normalMapIntensity;

    // transform normal into world space
    N = normalize(v_tbn * N);

    // return normal
    return N;
}

// get albedo color
vec3 getAlbedo()
{
    // default albedo is white
    vec3 albedo = vec3(1.0);

    // sample albedo map if enabled
    if (material.enableAlbedoMap) {
        vec3 albedoSample = texture(material.albedoMap, uv).rgb;
        albedo = pow(albedoSample, vec3(configuration.gamma));
    }

    // tint albedo by materials base color
    albedo *= vec3(material.baseColor);

    // return final albedo
    return albedo;
}

// get roughness value
float getRoughness()
{
    // initialize roughness
    float roughness = 0.0;

    // roughness map enabled, sample roughness by roughness map
    if (material.enableRoughnessMap) {
        roughness = texture(material.roughnessMap, uv).r;
        // no roughness map, set to materials roughness property
    } else {
        roughness = material.roughness;
    }

    // return roughness
    return roughness;
}

// get metallic value
float getMetallic()
{
    // initialize metallic
    float metallic = 0.0;

    // metallic map enabled, sample metallic by metallic map
    if (material.enableMetallicMap) {
        metallic = texture(material.metallicMap, uv).r;
        // no metallic map, set to materials metallic property
    } else {
        metallic = material.metallic;
    }

    // return metallic
    return metallic;
}

// get occlusion map sample value
float getOcclusionMapSample()
{
    // initialize occlusion map sample with no occlusion
    float occlusionMapSample = 1.0;

    // occlusion map enabled, sample by occlusion map
    if (material.enableOcclusionMap) {
        occlusionMapSample = texture(material.occlusionMap, uv).r;
    }

    // return occlusion map sample
    return occlusionMapSample;
}

// get ssao sample value
float getSSAO() {
    // initialize ssao sample with no occlusion
    float ssao = 1.0;

    // ssao enabled, sample by ssao buffer
    if (configuration.enableSSAO) {
        ssao = texture(configuration.ssaoBuffer, viewportUv).r;
    }

    // return ssao sample
    return ssao;
}

// get emission color
vec3 getEmission() {
    // return zero if emission isnt enabled
    if (!material.emission) {
        return vec3(0.0);
    }

    // get emission by intensity and color
    vec3 emission = vec3(material.emissionIntensity) * material.emissionColor;

    // emissive map enabled, tint emission by emissive map sample
    if (material.enableEmissiveMap) {
        emission *= texture(material.emissiveMap, uv).rgb;
    }

    // return emission
    return emission;
}

//
// ATTENUATION CALCULATION
//

float getAttenuation_linear_quadratic(float distance, float linear, float quadratic)
{
    float attenuation = 1.0 / (1.0 + linear * distance + quadratic * (distance * distance));
    return attenuation;
}
float getAttenuation_range_infinite(float distance, float range)
{
    float attenuation = 1.0 / (1.0 + sqr((distance / range)));
    return attenuation;
}
float getAttenuation_range_falloff_no_cusp(float distance, float range, float falloff)
{
    float s = distance / range;
    if (s >= 1.0) {
        return 0.0;
    }
    float s2 = sqr(s);
    return sqr(1 - s2) / (1 + falloff * s2);
}
float getAttenuation_range_falloff_cusp(float distance, float range, float falloff)
{
    float s = distance / range;
    if (s >= 1.0) {
        return 0.0;
    }
    float s2 = sqr(s);
    return sqr(1 - s2) / (1 + falloff * s);
}

//
// DEFAULT SHADING STYLES
//

vec4 shadePBR() {
    // get albedo
    vec3 albedo = getAlbedo();

    // get roughness
    float roughness = getRoughness();

    // get metallic
    float metallic = getMetallic();

    // get ambient occlusion
    float occlusionMapSample = getOcclusionMapSample();

    // get ssao
    float ssao = getSSAO();

    vec3 N = normal;
    vec3 V = normalize(configuration.cameraPosition - v_fragmentWorldPosition); // view direction

    float dialectricReflecitivity = 0.04;
    vec3 F0 = mix(vec3(dialectricReflecitivity), albedo, metallic); // base reflectivity

    // final light reflection or emission of fragment
    vec3 Lo = vec3(0.0);

    // get emission and check if fragment has emission
    vec3 emission = getEmission();

    // fragment has emission and therefore emits light
    if (emission.r > 1.0 || emission.g > 1.0 || emission.b > 1.0) {
        Lo = emission;
    }

    // fragment doesnt have emission and therefore reflects light from light sources
    else {

        //
        // DIRECTIONAL LIGHTS
        //

        for (int i = 0; i < configuration.numDirectionalLights; i++)
        {
            DirectionalLight directionalLight = directionalLights[i];

            float attenuation = 1.0;
            vec3 L = normalize(-directionalLight.direction);

            float shadow = 0.0;
            //shadow += getShadowSoft(L);
           
            // PARALLAX OCCLUSION MAPPED SHADOW FOR DIRECTIONAL LIGHT //
            /*if (material.enableHeightMap && i == 0) {
                vec3 tangentLightDirection = normalize(v_tbnTransposed * v_fragmentWorldPosition - v_tbnTransposed * directionalLight.position);
                shadow += POM_sampleShadow(tangentLightDirection, vec2(0.0));
            }*/

            Lo += evaluateLightSource(
                    V,
                    N,
                    F0,
                    roughness,
                    metallic,
                    albedo,
                    attenuation,
                    L,
                    directionalLight.color,
                    directionalLight.intensity,
                    shadow);
        }

        //
        // POINT LIGHTS
        //

        for (int i = 0; i < configuration.numPointLights; i++) {
            PointLight pointLight = pointLights[i];

            float distance = length(pointLight.position - v_fragmentWorldPosition);
            float attenuation = getAttenuation_range_falloff_cusp(distance, pointLight.range, pointLight.falloff);
            vec3 L = normalize(pointLight.position - v_fragmentWorldPosition);

            float shadow = 0.0;
            
            // PARALLAX OCCLUSION MAPPED SHADOW FOR POINT LIGHT //
            /*if (material.enableHeightMap && i == 0) {
                shadow += POM_getShadowHard(pointLight.position);
            }*/

            Lo += evaluateLightSource(
                    V,
                    N,
                    F0,
                    roughness,
                    metallic,
                    albedo,
                    attenuation,
                    L,
                    pointLight.color,
                    pointLight.intensity,
                    shadow);
        }

        //
        // SPOT LIGHTS
        //

        for (int i = 0; i < configuration.numSpotLights; i++) {
            Spotlight spotlight = spotlights[i];

            float distance = length(spotlight.position - v_fragmentWorldPosition);
            float attenuation = getAttenuation_range_falloff_cusp(distance, spotlight.range, spotlight.falloff);
            vec3 L = normalize(spotlight.position - v_fragmentWorldPosition);

            float theta = dot(L, normalize(-spotlight.direction));
            float epsilon = spotlight.innerCos - spotlight.outerCos;
            float intensityScaling = clamp((theta - spotlight.outerCos) / epsilon, 0.0, 1.0);

            float shadow = 0.0;
            shadow += getShadowSoft(L);
           
            // PARALLAX OCCLUSION MAPPED SHADOW FOR SPOT LIGHT //
            /*if (material.enableHeightMap && i == 0) {
                shadow += POM_getShadowHard(spotlight.position);
            }*/

            Lo += evaluateLightSource(
                    V,
                    N,
                    F0,
                    roughness,
                    metallic,
                    albedo,
                    attenuation,
                    L,
                    spotlight.color,
                    spotlight.intensity * intensityScaling,
                    shadow);
        }
    }

    // assemble shaded color
    vec3 color = Lo;

    // apply ambient occlusion: modulate color by ambient occlusion map sample and ssao
    color *= occlusionMapSample * ssao;

    // gamma correct if using albedo map
    if (material.enableAlbedoMap) {
        color = pow(color, vec3(1.0 / configuration.gamma));
    }

    // get fog
    float fogFactor = 1.0;
    switch (fog.type) {
        case LINEAR_FOG:
        fogFactor = getLinearFog(fog.data[0], fog.data[1]);
        break;
        case EXPONENTIAL_FOG:
        fogFactor = getExponentialFog(fog.data[0]);
        break;
        case EXPONENTIAL_SQUARED_FOG:
        fogFactor = getExponentialSquaredFog(fog.data[0]);
        break;
    }
    color = mix(fog.color, color, fogFactor);

    // return shaded color
    return vec4(color, 1.0);
}
vec4 shadeSolid()
{
    vec3 diffuse = vec3(0.0);

    vec3 N = normal;

    // static directional light
    vec3 direction = vec3(-0.5, -0.5, 1.0);

    for (int i = 0; i < configuration.numDirectionalLights; i++)
    {
        DirectionalLight directionalLight = directionalLights[i];

        float attenuation = 1.0;
        vec3 L = normalize(-directionalLight.direction);

        vec3 shadowDirection = normalize(directionalLight.position - v_fragmentWorldPosition);
        float shadow = getShadowHard(L);

        diffuse += max(dot(N, L), 0.0) * directionalLight.color * directionalLight.intensity * attenuation * (1.0 - shadow);
    }

    vec3 albedo = vec3(1.0);
    if (material.enableAlbedoMap) {
        albedo = texture(material.albedoMap, uv).rgb;
    }
    albedo *= vec3(material.baseColor);

    vec3 color = diffuse * albedo;

    return vec4(color, 1.0);
}
vec4 shadeNormal() {
    // get normal
    vec3 colorNormal = normal;

    // remap normal range from [-1, 1] to [0, 1]
    colorNormal = (colorNormal + 1.0) * 0.5;

    // if available, sample diffuse lighting and shadow from first directional light for depth enhancement
    vec3 diffuse = vec3(1.0);
    float diffuseFactor = 0.3;
    float shadow = 0.0;
    float shadowIntensity = 0.5;

    // static directional light
    DirectionalLight directionalLight;
    directionalLight.direction = vec3(-0.5, -0.5, 1.0);
    vec3 L = normalize(directionalLight.direction);

    diffuse = vec3(max(dot(normal, L), 0.0));
    diffuse = mix(diffuse, vec3(0.0), diffuseFactor);

    shadow = getShadowSoft(L) * shadowIntensity;

    // get color from normal and shadow
    vec3 color = colorNormal * diffuse * (1.0 - shadow);

    // return normal as color
    return vec4(color, 1.0);
}
vec4 shadeDepth() {
    // camera clipping default variables
    float near = 0.3;
    float far = 1000;

    // get depth
    float z = gl_FragCoord.z * 2.0 - 1.0;
    float depth = ((2.0 * near * far) / (far + near - z * (far - near))) / far;

    // retun depth as color
    return vec4(vec3(depth), 1.0);
}
vec4 shadeShadowMap() {
    vec3 projectionCoordinates = v_fragmentLightSpacePosition.xyz / v_fragmentLightSpacePosition.w;
    projectionCoordinates = projectionCoordinates * 0.5 + 0.5;
    float depth = texture(configuration.shadowMap, projectionCoordinates.xy).r;
    return vec4(vec3(depth), 1.0);
}
vec4 shadeUv(){
    return vec4(uv, 0.0, 1.0);
}
vec4 shadeTangentSpace() {
    // extract the tangent, bitangent, and normal from tbn matrix
    vec3 tangent = normalize(v_tbn[0]);
    vec3 bitangent = normalize(v_tbn[1]);
    vec3 normal = normalize(v_tbn[2]);

    // map vectors to [0, 1] range for visualization
    vec3 tangentColor = (tangent * 0.5) + 0.5;
    vec3 bitangentColor = (bitangent * 0.5) + 0.5;
    vec3 normalColor = (normal * 0.5) + 0.5;

    return vec4(tangentColor.r, bitangentColor.g, normalColor.b, 1.0);
}

void main()
{
    viewportUv = gl_FragCoord.xy / vec2(configuration.viewportResolution.x, configuration.viewportResolution.y);
    uv = getUv();
    normal = getNormal();

    if (!configuration.solidMode) {
        FragColor = shadePBR();
    } else {
        FragColor = shadeSolid();
    }
}