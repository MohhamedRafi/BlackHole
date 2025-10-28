#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aCol;
out vec3 vCol;
uniform mat4 uTransform; // same uniform your Renderer already sets
void main() {
    vCol = aCol;
    gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
}