#version 330 core

out vec4 FragColor;

uniform sampler2D hdrInput;
uniform sampler2D depthInput;

uniform float near;
uniform float far;

uniform float fps;

uniform bool camera;
uniform float cameraIntensity;
uniform int cameraSamples;

uniform mat4 inverseViewProjectionMatrix;
uniform mat4 previousViewProjectionMatrix;

in vec2 uv;

vec3 getWorldPosition(float depth) {
    // get fragment position in clip space (normalize texture coordinates and depth to NDC)
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // transform clip space position to world space position
    vec4 worldSpacePosition = inverseViewProjectionMatrix * clipSpacePosition;

    // perspective division
    worldSpacePosition /= worldSpacePosition.w;

    // return world space position
    return worldSpacePosition.xyz;
}

vec4 cameraMotionBlur(vec4 color) {
    // get fragments current position in world space
    vec3 currentWorldPosition = getWorldPosition(texture(depthInput, uv).r);

    // get fragments previous position in screen space
    vec4 previousScreenPosition = previousViewProjectionMatrix * vec4(currentWorldPosition, 1.0);
    previousScreenPosition.xyz /= previousScreenPosition.w;
    previousScreenPosition.xy = previousScreenPosition.xy * 0.5 + 0.5;

    // calculate scale for blur direction to compensate varying framerates
    float blurScale = fps / 60;

    // calculate direction for motion blur
    vec2 blurDirection = previousScreenPosition.xy - uv;
    blurDirection *= blurScale;

    // perform motion blur on input
    for (int i = 1; i < cameraSamples; ++i) {
        // get blur offset
        vec2 offset = blurDirection * (float(i) / float(cameraSamples - 1) - 0.5) * cameraIntensity;
        // sample iteration
        color += texture(hdrInput, uv + offset);
    }
    // average accumulated samples to motion blur input
    color /= float(cameraSamples);

    // return motion blurred input
    return color;
}

void main() {
    vec4 color = texture(hdrInput, uv);

    // perform camera motion blur
    if (camera) {
        color = cameraMotionBlur(color);
    }

    FragColor = color;
}
