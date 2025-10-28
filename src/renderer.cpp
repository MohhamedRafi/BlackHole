#include "renderer.h"
#include "shader_library.h"
#include <cstdio> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool Renderer::init_triangle(ShaderLibrary& lib) {
  // Get shader program
  prog = lib.get_flat_color().id;
  if (!prog) return false;

  uTransformLoc = glGetUniformLocation(prog, "uTransform");
  if (uTransformLoc < 0) std::fprintf(stderr, "warn: uTransform not found\n");

  // Simple triangle: 2D position (x,y) +  RGB color per-vertex
  // Interleaved layout: [x, y, r, g, b]
  const float verts[] = {
    //    x      y      r     g     b
    -0.6f, -0.5f,   1.0f, 0.2f, 0.2f,
      0.6f, -0.5f,   0.2f, 1.0f, 0.2f,
      0.0f,  0.6f,   0.2f, 0.2f, 1.0f,
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  if (!vao || !vbo) return false;
  
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  // layout(location=0) in vec2 aPos;
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  // layout(location=1) in vec3 aCol;
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return true;
}

void Renderer::shutdown() {
  if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0;} 
  if (vao) { glDeleteVertexArrays(1, &vao); vao = 0;} 
}

void Renderer::draw(float angle_radians, const glm::mat4& VP) {
    if (!prog || !vao) return;
     glUseProgram(prog);

    glm::mat4 M(1.0f);
    M = glm::rotate(M, angle_radians, glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::mat4 MVP = VP * M;
    if (uTransformLoc >= 0)
        glUniformMatrix4fv(uTransformLoc, 1, GL_FALSE, glm::value_ptr(MVP));

        
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);
}