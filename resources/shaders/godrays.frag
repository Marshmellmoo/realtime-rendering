#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D occlusionTexture;

struct BlurParameters {
    int sampleCount;
    float blurDensity;
    float blurExposure;
    float decayFactor;
    float sampleWeight;
};

uniform bool enableBlur = true;
uniform BlurParameters blurParams;
uniform vec4 lightPositionsScreen[8];
uniform int lightCount;
uniform float blendAlpha = 0.3;

// Forward declarations
vec3 sampleRadialBlur(BlurParameters params, vec2 lightScreenPos);
vec3 accumulateBlur(BlurParameters params);

void main() {
    fragColor = vec4(0.0);
    if (enableBlur) {
        vec3 blurResult = accumulateBlur(blurParams);
        fragColor = vec4(blurResult, 1.0);
    } else {
        fragColor = texture(occlusionTexture, fragTexCoord);
    }

    fragColor = vec4(1.0, 0.0, 1.0, 1.0);
}

vec3 sampleRadialBlur(BlurParameters params, vec2 lightScreenPos) {
    vec2 rayDirection = (fragTexCoord - lightScreenPos) * params.blurDensity * (1.0 / float(params.sampleCount));
    
    vec2 sampleCoord = fragTexCoord;
    vec3 accumulatedColor = texture(occlusionTexture, sampleCoord).rgb;
    float decayValue = 1.0;
    
    for (int i = 0; i < params.sampleCount; ++i) {
        sampleCoord -= rayDirection;
        vec3 sampleValue = texture(occlusionTexture, sampleCoord).rgb;
        sampleValue *= decayValue * params.sampleWeight;
        accumulatedColor += sampleValue;
        decayValue *= params.decayFactor;
    }

    return accumulatedColor * params.blurExposure;
}

vec3 accumulateBlur(BlurParameters params) {
    vec3 totalBlur = vec3(0.0);
    
    for (int i = 0; i < lightCount; ++i) {
        totalBlur += sampleRadialBlur(params, lightPositionsScreen[i].xy);
    }

    return totalBlur;
}

