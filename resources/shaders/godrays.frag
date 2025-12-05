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

uniform BlurParameters blurParams;
uniform vec4 lightPositionsScreen[8];
uniform int lightCount;

vec3 sampleRadialBlur(BlurParameters params, vec2 lightScreenPos) {

    vec2 delta_tex_coord = (fragTexCoord - lightScreenPos) * params.blurDensity * (1.0 / float(params.sampleCount));
    vec2 tex_coordinates = fragTexCoord;
    vec3 color = texture(occlusionTexture, tex_coordinates).rgb;
    float decay = 1.0;

    for (int i = 0; i < params.sampleCount; ++i) {

        tex_coordinates -= delta_tex_coord;
        vec3 current_sample = texture(occlusionTexture, tex_coordinates).rgb;
        current_sample *= decay * params.sampleWeight;
        color += current_sample;
        decay *= params.decayFactor;

    }

    return color * params.blurExposure;
}

vec3 accumulateBlur(BlurParameters params) {

    vec3 multiple_sources_color = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {

        multiple_sources_color += sampleRadialBlur(params, lightPositionsScreen[i].xy);

    }

    return multiple_sources_color;
}

void main() {

    fragColor = vec4(0.0);

    if (lightCount > 0) {

        fragColor = vec4(accumulateBlur(blurParams), 1.0);

    } else {

        fragColor = texture(occlusionTexture, fragTexCoord);

    }
}
