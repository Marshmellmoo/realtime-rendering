#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D depthTexture;
uniform float minDist;
uniform float maxDist;
uniform vec3 fogColour;

void main() {

    vec4 sceneColor = texture(sceneTexture, fragTexCoord);
    float depth = texture(depthTexture, fragTexCoord).r;

    float dist = mix(minDist, maxDist, depth);

    float fogFactor = (dist - minDist) / (maxDist - minDist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 result = mix(sceneColor.rgb, fogColour, fogFactor);

    fragColor = vec4(result, sceneColor.a);

}
