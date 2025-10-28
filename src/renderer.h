#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader_library.h" 

struct Renderer {
  GLuint prog = 0, vao = 0, vbo = 0, ebo = 0;;

  bool init_triangle(ShaderLibrary& lib);
  bool init_cube(ShaderLibrary& lib);


  int uTransformLoc = -1;                

  void draw(float angle_radians, const glm::mat4& VP);
  void shutdown();
};