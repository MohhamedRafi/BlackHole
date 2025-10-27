#include "shader_library.h"
#include <cstdio>

void ShaderLibrary::shutdown() {
    for (auto& [_, sp] : progs) {
        if (sp.id) { glDeleteProgram(sp.id); sp.id = 0; }
    }
    progs.clear();
}

/* Flat Color Example */

static const char* VS_FLAT = R"(#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aCol;
out vec3 vCol;
uniform mat4 uTransform;            // NEW
void main(){
    vCol = aCol;
    gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
})";

static const char* FS_FLAT = R"(#version 330 core
in vec3 vCol; out vec4 fragColor;
void main(){ fragColor = vec4(vCol, 1.0); })";

const ShaderProgram& ShaderLibrary::get_flat_color() {
    auto it = progs.find("flat");
    if (it != progs.end()) return it->second;

    std::string err;
    GLuint vs = compile_shader(GL_VERTEX_SHADER, VS_FLAT, &err);
    if (!vs) std::fprintf(stderr, "VS error: %s\n", err.c_str());
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FS_FLAT, &err);
    if (!fs) std::fprintf(stderr, "FS error: %s\n", err.c_str());
    GLuint prog = link_program(vs, fs, &err);
    if (!prog) std::fprintf(stderr, "Link error: %s\n", err.c_str());

    ShaderProgram sp{prog};
    auto [ins, _] = progs.emplace("flat", sp);
    return ins->second;
}