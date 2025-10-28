#pragma once 
#include <glad/glad.h>
#include <string>

struct ShaderProgram {
  GLuint id = 0;
  void use() const { glUseProgram(id); }
}; 

GLuint compile_shader(GLenum type, const char* src, std::string* err = nullptr); 
GLuint link_program(GLuint vs, GLuint fs, std::string* err = nullptr);
GLuint compile_shader_file(GLenum type, const char* filepath_rel, std::string* err = nullptr);