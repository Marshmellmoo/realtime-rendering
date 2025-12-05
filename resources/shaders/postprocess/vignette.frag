#version 330 core

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform float vignetteStrength;  // Default: 0.5
uniform float vignetteExtent;    // Default: 0.5

void main() {

    vec3 color = texture(inputTexture, fragTexCoord).rgb;
    
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(fragTexCoord, center);
    
    float vignette = smoothstep(vignetteExtent, vignetteExtent - vignetteStrength, dist);
    
    color *= vignette;
    
    fragColor = vec4(color, 1.0);
    
}
