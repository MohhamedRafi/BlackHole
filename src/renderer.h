#pragma once

#include <glad/glad.h>
#include "shader_library.h" 

struct Renderer {
  GLuint prog = 0, vao = 0, vbo = 0;

  bool init_triangle(ShaderLibrary& lib);

  int uTransformLoc = -1;                

  void draw(float angle_radians);
  void shutdown();
};