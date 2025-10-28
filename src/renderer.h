#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader_library.h" 

struct Renderer {
  GLuint prog = 0, vao = 0, vbo = 0, ebo = 0;
  GLuint rmProg = 0, fsVAO = 0, fsVBO = 0;


  bool init_triangle(ShaderLibrary& lib);
  bool init_cube(ShaderLibrary& lib);
  bool init_raymarch(ShaderLibrary& lib);

  int uTransformLoc = -1;                
  int uInvVPLoc = -1, uTimeLoc = -1, uResLoc = -1, uCamPosLoc = -1; // RM core


  void draw(float angle_radians, const glm::mat4& VP);
  void draw_raymarch(double time_sec, const glm::mat4& VP, const glm::vec3& camPos, int width, int hight);
 
  void shutdown();
};