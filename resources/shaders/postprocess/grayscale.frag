#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D inputTexture;

// Grayscale conversion weights (luminance)
const vec3 luminanceWeights = vec3(0.2126, 0.7152, 0.0722);

void main() {

    vec3 color = texture(inputTexture, fragTexCoord).rgb;
    float gray = dot(color, luminanceWeights);
    
    fragColor = vec4(vec3(gray), 1.0);
    
}
