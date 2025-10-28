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

bool Renderer::init_cube(ShaderLibrary& lib) {
    prog = lib.get_flat_color().id;
    if (!prog) return false;

    uTransformLoc = glGetUniformLocation(prog, "uTransform");

    // position(x,y,z) + color(r,g,b)
    const float verts[] = {
        // Front
        -0.5f,-0.5f, 0.5f, 1,0,0,   0.5f,-0.5f, 0.5f, 0,1,0,
         0.5f, 0.5f, 0.5f, 0,0,1,  -0.5f, 0.5f, 0.5f, 1,1,0,
        // Back
        -0.5f,-0.5f,-0.5f, 1,0,1,   0.5f,-0.5f,-0.5f, 0,1,1,
         0.5f, 0.5f,-0.5f, 1,1,1,  -0.5f, 0.5f,-0.5f, 0,0,0,
    };
    // Consistent CCW winding cube indices
    const unsigned int idx[] = {
      0,1,2, 2,3,0,   // front
      5,4,7, 7,6,5,   // back
      4,0,3, 3,7,4,   // left
      1,5,6, 6,2,1,   // right
      3,2,6, 6,7,3,   // top
      4,5,1, 1,0,4    // bottom
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    if (!vao || !vbo || !ebo) return false;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);

    // layout(location=1) vec3 aCol
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

    glBindVertexArray(0);
    return true;
}

static const float FS_TRI[6] = {
  -1.0f, -1.0f,
   3.0f, -1.0f,
  -1.0f,  3.0f
};


bool Renderer::init_raymarch(ShaderLibrary& lib) {
  rmProg = lib.get_raymarch().id;
  if(!rmProg) return false;

  // uniforms
  uInvVPLoc  = glGetUniformLocation(rmProg, "uInvVP");
  uTimeLoc   = glGetUniformLocation(rmProg, "uTime");
  uResLoc    = glGetUniformLocation(rmProg, "uResolution");
  uCamPosLoc = glGetUniformLocation(rmProg, "uCameraPos");
  
  // full-screen triangle
  glGenVertexArrays(1, &fsVAO);
  glGenBuffers(1, &fsVBO);
  glBindVertexArray(fsVAO);
  glBindBuffer(GL_ARRAY_BUFFER, fsVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(FS_TRI), FS_TRI, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
  glBindVertexArray(0);
  return true;

}


void Renderer::shutdown() {
  if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0;} 
  if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0;}
  if (vao) { glDeleteVertexArrays(1, &vao); vao = 0;} 
  if (fsVBO) { glDeleteBuffers(1, &fsVBO); fsVBO = 0; }
  if (fsVAO) { glDeleteVertexArrays(1, &fsVAO); fsVAO = 0; }
}

void Renderer::draw(float angle_radians, const glm::mat4& VP) {
    if (!prog || !vao) return;
    glUseProgram(prog);

    glm::mat4 M(1.0f);
    M = glm::rotate(M, angle_radians, glm::vec3(0.0f, 1.0f, 0.0f)); // Y
    M = glm::rotate(M, angle_radians * 0.7f, glm::vec3(1.0f, 0.0f, 0.0f)); // X
    const glm::mat4 MVP = VP * M;
    if (uTransformLoc >= 0)
        glUniformMatrix4fv(uTransformLoc, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Renderer::draw_raymarch(double time_sec, const glm::mat4& VP, const glm::vec3& camPos, int width, int height) {
  if (!rmProg || !fsVAO) return;
  glUseProgram(rmProg);
  // Send inverse VP for ray reconstruction
  glm::mat4 invVP = glm::inverse(VP);
  if (uInvVPLoc >= 0) glUniformMatrix4fv(uInvVPLoc, 1, GL_FALSE, glm::value_ptr(invVP));
  if (uTimeLoc  >= 0) glUniform1f(uTimeLoc,  (float)time_sec);
  if (uResLoc   >= 0) glUniform2f(uResLoc,   (float)width, (float)height);
  if (uCamPosLoc>= 0) glUniform3fv(uCamPosLoc, 1, glm::value_ptr(camPos));
  glBindVertexArray(fsVAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glUseProgram(0);
}