#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform mat4 uTransform;

out vec3 vCol;

void main() {
    vCol = aCol;
    gl_Position = uTransform * vec4(aPos, 1.0);
}
