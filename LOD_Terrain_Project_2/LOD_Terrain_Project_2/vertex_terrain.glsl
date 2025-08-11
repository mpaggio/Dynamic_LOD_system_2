#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 inDisplace;

out vec4 vDisplace;

void main() {
    gl_Position = vec4(aPos.x, aPos.z, aPos.y, 1.0);
    vDisplace = inDisplace;
}