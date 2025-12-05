#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform vec2 screenSize;
uniform float blurRadius;  // Default: 1.0

// Gaussian kernel for 5x5 blur
const float kernel[25] = float[](
    1.0/256.0,  4.0/256.0,  6.0/256.0,  4.0/256.0, 1.0/256.0,
    4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0,
    6.0/256.0, 24.0/256.0, 36.0/256.0, 24.0/256.0, 6.0/256.0,
    4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0,
    1.0/256.0,  4.0/256.0,  6.0/256.0,  4.0/256.0, 1.0/256.0
);

void main() {
    vec2 texelSize = 1.0 / screenSize * blurRadius;
    vec3 result = vec3(0.0);
    
    int index = 0;
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {

            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(inputTexture, fragTexCoord + offset).rgb * kernel[index];
            index++;
            
        }
    }
    
    fragColor = vec4(result, 1.0);
}
