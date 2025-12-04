#version 330 core

uniform sampler2D depth;
uniform float minDist;
uniform float maxDist;
uniform vec4 fogColour;

void main() {

    // float dist = length(fposition.xyz);
    // float fog_factor = (maxDist - gl_FragCoord.z) / (maxDist - minDist);
    // fog_factor = clamp(fog_factor, 0.0, 1.0);

    // outputColor = mix(fogColour, shadedColor, fogColour);

}
