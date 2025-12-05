#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

// Instanced model matrix (4 vec4s = mat4)
layout(location = 2) in vec4 instanceModel0;
layout(location = 3) in vec4 instanceModel1;
layout(location = 4) in vec4 instanceModel2;
layout(location = 5) in vec4 instanceModel3;

// Instanced material properties
layout(location = 6) in vec4 instanceAmbient;
layout(location = 7) in vec4 instanceDiffuse;
layout(location = 8) in vec4 instanceSpecular;
layout(location = 9) in float instanceShininess;

uniform mat4 view;
uniform mat4 proj;

out vec3 worldSpacePosition;
out vec3 worldSpaceNormal;
out vec4 materialAmbient;
out vec4 materialDiffuse;
out vec4 materialSpecular;
out float materialShininess;

void main() {
    
    mat4 model = mat4(instanceModel0, instanceModel1, instanceModel2, instanceModel3);
    mat4 invModel = transpose(inverse(model));

    vec4 worldPosition = model * vec4(position, 1.0f);
    worldSpacePosition = worldPosition.xyz;

    worldSpaceNormal = vec3(invModel * vec4(normal, 0.0));

    materialAmbient = instanceAmbient;
    materialDiffuse = instanceDiffuse;
    materialSpecular = instanceSpecular;
    materialShininess = instanceShininess;

    gl_Position = proj * view * worldPosition;

}
