#version 330 core

out vec4 fragColor;

// This shader renders geometry as black and light sources as white for the occlusion map
uniform vec4 occlusionColor;

void main() {

    fragColor = occlusionColor;
    
}
