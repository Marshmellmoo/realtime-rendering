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

    // Linear fog: f = (end - distance) / (end - start)
    // where distance is the actual depth value from the depth texture
    // Remap normalized depth [0, 1] to world space approximation
    float distance = mix(minDist, maxDist, depth);

    // Linear fog factor: starts fully opaque at minDist, fades to fully foggy at maxDist
    float fogFactor = clamp((maxDist - distance) / (maxDist - minDist), 0.0, 1.0);

    // Blend scene color with fog color based on fog factor
    // fogFactor = 1.0 means no fog (close to camera)
    // fogFactor = 0.0 means full fog (far from camera)
    vec3 result = mix(fogColour, sceneColor.rgb, fogFactor);

    fragColor = vec4(result, sceneColor.a);

}
