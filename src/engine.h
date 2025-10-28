#pragma once 

#include <cstdint>
#include <queue>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "shader_library.h"
#include "camera.h"


struct WindowEvent {
  enum Type {Close, Resize, FocusLost, FocusGained, KeyDown, KeyUp} type; 

  int a = 0; int b = 0; /* modifers for event type */
};

enum class EngineState : uint8_t { 
  Boot, InitGL, Loading, Running, Paused, Suspended, ShuttingDown 
};

struct Engine {
  bool running = true; 
  int width = 800; int height = 600;

  EngineState state = EngineState::Boot;
  GLFWwindow* window = nullptr;

  ShaderLibrary shaders; 
  Renderer renderer;
  Camera camera;

  bool captureMouse = true;
  bool firstMouse = true;
  double lastMouseX = 0.0, lastMouseY = 0.0; 

  void setup_input_callbacks();


  double time_now = 0.0, time_prev = 0.0, accumulator = 0.0;
  static constexpr double DT = 1.0 / 120;

  float angle = 0.0f; 
  float angular_velocity = 1.0f; 
  

  std::queue<WindowEvent> events;

  bool on_enter(EngineState s);
  void on_exit(EngineState s);
  void update_fixed(double dt);
  void update_variable(double dt);
  void render();
  void process_events();

  void go(EngineState next); 
  void push_event(const WindowEvent& e) { events.push(e);}
};