#include "shader.h"
#include "asset_loader.h"
#include <string>


GLuint compile_shader(GLenum type, const char* src, std::string* err) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; GLsizei n=0; glGetShaderInfoLog(s, 1024, &n, log);
        if (err) *err = std::string(log, n);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint link_program(GLuint vs, GLuint fs, std::string* err) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    glDetachShader(p, vs); glDetachShader(p, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; GLsizei n=0; glGetProgramInfoLog(p, 1024, &n, log);
        if (err) *err = std::string(log, n);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLuint compile_shader_file(GLenum type, const char* filepath_rel, std::string* err) {
    std::string src; 
    if(!AssetLoader::instance().read_text(filepath_rel, src)) {
        if (err) *err = std::string("Could not read life: ") + filepath_rel;
        return 0;
    }
    return compile_shader(type, src.c_str(), err);
}