#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D depthTexture;
uniform float minDist;
uniform float maxDist;
uniform vec3 fogColour;
uniform float nearPlane;
uniform float farPlane;
uniform bool useLinear;   // true = linear fog, false = exponential fog
uniform float fogDensity; // density for exponential fog

// Convert non-linear depth buffer value to linear view-space depth
float linearizeDepth(float depth) {

    float z_ndc = depth * 2.0 - 1.0;
    float z_eye = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z_ndc * (farPlane - nearPlane));
    return abs(z_eye);

}

void main() {

    vec4 sceneColor = texture(sceneTexture, fragTexCoord);
    float depth = texture(depthTexture, fragTexCoord).r;

    float distance = linearizeDepth(depth);
    float fogFactor;

    if (useLinear) {

        fogFactor = clamp((maxDist - distance) / (maxDist - minDist), 0.0, 1.0);

    } else {

        fogFactor = exp(-fogDensity * distance);
    }

    vec3 result = mix(fogColour, sceneColor.rgb, fogFactor);
    fragColor = vec4(result, 1.0);
}
