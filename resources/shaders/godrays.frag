#version 330 core
in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D occlusionTexture;
uniform vec2 lightScreenPositions[8];
uniform int numSamples;
uniform float density;
uniform float weight;
uniform float decay;
uniform float exposure;

void main() {
    vec2 deltaTexCoord = (fragTexCoord - lightScreenPositions[0]);
    deltaTexCoord *= density / float(numSamples);

    vec2 texCoord = fragTexCoord;
    float illuminationDecay = 1.0;
    vec3 godRays = vec3(0.0);

    for (int i = 0; i < numSamples; i++) {
        texCoord -= deltaTexCoord;
        vec3 sampleColor = texture(occlusionTexture, texCoord).rgb;
        sampleColor *= illuminationDecay * weight;
        godRays += sampleColor;
        illuminationDecay *= decay;
    }

    godRays *= exposure;
    // fragColor = vec4(godRays, 1.0);
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);

}
