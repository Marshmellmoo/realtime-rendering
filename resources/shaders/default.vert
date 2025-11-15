#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 worldSpacePosition;
out vec3 worldSpaceNormal;

void main() {

    vec4 worldPosition = model * vec4(position, 1.0f);
    worldSpacePosition = worldPosition.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model))); // compute in c++
    worldSpaceNormal = normalMatrix * normal;

    gl_Position = proj * view * worldPosition;

}
