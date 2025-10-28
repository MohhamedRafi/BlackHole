#pragma once 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * =====================================================
 * Camera System
 * -----------------------------------------------------
 * Provides the View (V) and Projection (P) matrices
 * used in the classic graphics transform pipeline:
 *
 *     clip_position = P * V * M * local_position
 *
 * Where:
 *   - M = Model matrix (object's local transform)
 *   - V = View matrix   (camera's inverse transform)
 *   - P = Projection matrix (perspective or orthographic)
 *
 * The Camera is defined by:
 *   - position: the camera's location in world space
 *   - target:   the point the camera is looking at
 *   - up:       the camera's "up" vector, defining roll
 * =====================================================
 */

struct Camera {
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 front{0.0f, 0.0f, -1.0f};    // view direction (derived from yaw/pitch)
  glm::vec3 up{0.0f, 1.0f, 0.0f}; 

  float fov = 45.0f; 
  float aspect = 16.0f/9.0f; 
  float nearPlane = 0.1f; 
  float farPlane = 100.0f; 

  
  float yaw = -90.0f; 
  float pitch = 0.0f; 
  float moveSpeed = 3.0f; 
  float mouseSensitivity = 0.1f; 

  /**
 * View Matrix (V)
 * ----------------
 * Converts coordinates from world space → camera space.
 *
 * Built using glm::lookAt(), which internally constructs
 * the camera basis vectors:
 *   - z_cam = normalize(eye - target)      // forward (camera's -Z)
 *   - x_cam = normalize(cross(up, z_cam))  // right
 *   - y_cam = cross(z_cam, x_cam)          // up (orthogonalized)
 *
 * The resulting matrix moves the world so that the camera
 * sits at the origin and looks down -Z.
 */
  glm::mat4 getView() const {
    return glm::lookAt(position, position + front, up); 
  }

  /**
   * Projection Matrix (P)
   * ----------------------
   * Converts coordinates from camera space → clip space.
   * This defines the viewing frustum for perspective projection.
   *
   * The standard perspective matrix:
   *   [ 1/(tan(fov/2)*aspect)   0              0                 0 ]
   *   [ 0                       1/tan(fov/2)   0                 0 ]
   *   [ 0                       0     -(f+n)/(f-n)  -2fn/(f-n)   ]
   *   [ 0                       0              -1                0 ]
   *
   * where f = farPlane, n = nearPlane.
   *
   * The nonlinear division by w (done automatically by OpenGL)
   * produces depth perception — objects farther away appear smaller.
   */
  glm::mat4 getProj() const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
  }

  /**
   * Combined View-Projection (VP)
   * ------------------------------
   * Often used directly in shaders to transform world-space
   * positions into clip space:
   *
   *    clip = P * V * M * local_position
   */
  glm::mat4 getViewProj() const {
      return getProj() * getView();
  }

  // Rebuild 'front' from yaw/pitch (in degrees).
  void updateVectors() {
      const float cy = cos(glm::radians(yaw));
      const float sy = sin(glm::radians(yaw));
      const float cp = cos(glm::radians(pitch));
      const float sp = sin(glm::radians(pitch));
      front = glm::normalize(glm::vec3(cy * cp, sp, sy * cp) * glm::vec3(1,1,-1)); // OpenGL -Z forward
      // keep 'up' as world up (0,1,0); if you want roll, orthonormalize here.
  }

  // Mouse delta in pixels (dx, dy); positive dy means cursor moved up.
  void processMouse(float dx, float dy) {
      yaw   += dx * mouseSensitivity;
      pitch -= dy * mouseSensitivity;          // invert to feel natural
      pitch  = glm::clamp(pitch, -89.0f, 89.0f);
      updateVectors();
  }

  // Keyboard intent: forward/back/left/right
  void processKeyboard(bool fwd, bool back, bool left, bool right, float dt) {
      const float v = moveSpeed * dt;
      const glm::vec3 rightVec = glm::normalize(glm::cross(front, up));
      if (fwd)  position += front * v;
      if (back) position -= front * v;
      if (left) position -= rightVec * v;
      if (right)position += rightVec * v;
  }

};