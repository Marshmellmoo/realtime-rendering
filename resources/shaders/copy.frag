#version 330 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D sceneTexture;

void main() {
    fragColor = texture(sceneTexture, texCoord);
}
