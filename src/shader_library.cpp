#include "shader_library.h"
#include <cstdio>

void ShaderLibrary::shutdown() {
    for (auto& [_, sp] : progs) {
        if (sp.id) { glDeleteProgram(sp.id); sp.id = 0; }
    }
    progs.clear();
}

const ShaderProgram& ShaderLibrary::get_from_files(const std::string& name, const std::string& vs_rel, const std::string& fs_rel) {
    auto it = progs.find(name); 
    if (it != progs.end()) return it->second;

    std::string err; 
    GLuint vs = compile_shader_file(GL_VERTEX_SHADER,   vs_rel.c_str(), &err);
    if (!vs) std::fprintf(stderr, "[%s] VS file error: %s\n", name.c_str(), err.c_str());
    GLuint fs = compile_shader_file(GL_FRAGMENT_SHADER, fs_rel.c_str(), &err);
    if (!fs) std::fprintf(stderr, "[%s] FS file error: %s\n", name.c_str(), err.c_str());

    GLuint prog = 0;
    if (vs && fs) {
        prog = link_program(vs, fs, &err); 
        if (!prog) if (!vs) std::fprintf(stderr, "[%s] Link error: %s\n", name.c_str(), err.c_str());
    } else {
        if (vs) glDeleteShader(vs); 
        if (fs) glDeleteShader(fs); 
   }

   ShaderProgram sp{prog};
   auto [ins, _] = progs.emplace(name, sp);
   return ins->second;
}

const ShaderProgram& ShaderLibrary::get_raymarch() {
    auto it = progs.find("raymarch");
    if (it != progs.end()) return it->second;

    {
        const ShaderProgram& rp = get_from_files("raymarch", "shaders/raymarch.vert", "shaders/blackhole.frag"); 
        if (rp.id) return rp;
    }

    return ShaderProgram();
}

/* Flat Color Example */
const ShaderProgram& ShaderLibrary::get_flat_color() {
    auto it = progs.find("flat");
    if (it != progs.end()) return it->second;
    {
        // find the files first
        const ShaderProgram& fp = get_from_files("flat", "shaders/flat.vert", "shaders/flat.frag");
        if(fp.id) return fp;
    }

    return ShaderProgram();
}
