#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
uniform mat4 depthMVP;
uniform sampler2D depth;

void main() {

    gl_Position = depthMVP * vec4(vertexPosition_modelspace, 1);

}
