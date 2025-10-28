#include "engine.h"
#include <iostream>

bool Engine::on_enter(EngineState s) {
  switch (s) {
    case EngineState::Boot:      std::cout << "[enter] Boot\n"; return true;
    case EngineState::InitGL:    {
      std::cout << "[enter] InitGL\n"; /* create window, GL ctx */ 

      if (!glfwInit()) {
        std::cout << "glfwIniti failed\n";
        return false; 
      }

      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); 

      window = glfwCreateWindow(width, height, "BlackHole", nullptr, nullptr); 
      if (!window) { 
        std::cout<< "glfwCreateWindow failed!\n";
        glfwTerminate();
        return false;
      }

      glfwMakeContextCurrent(window);
      glfwSwapInterval(1); 

      glfwSetWindowUserPointer(window, this); 
      if(captureMouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    
      } 
      setup_input_callbacks();


      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
          std::fprintf(stderr, "gladLoadGL failed\n");
          return false;
      }

      glViewport(0, 0, width, height);
      glfwShowWindow(window);

      return true;
    };
    
    case EngineState::Loading:   {
      std::cout << "[enter] Loading\n";

      if(!renderer.init_triangle(shaders)) {
        std::cout << "Renderer init triangle failed!\n"; 
        return false; 
      }
      
      camera.updateVectors();

      return true;
    };

    case EngineState::Running:   std::cout << "[enter] Running\n"; return true;
    case EngineState::Paused:    std::cout << "[enter] Paused\n"; return true;
    case EngineState::Suspended: std::cout << "[enter] Suspended\n"; return true;
    case EngineState::ShuttingDown: {
      std::cout << "[enter] Shutting Down\n"; 
      
      renderer.shutdown();
      shaders.shutdown();

      if(window) {
        glfwDestroyWindow(window); 
        window = nullptr;
      }
      glfwTerminate();

      break;
    };

  }

  return true;
}

void Engine::on_exit(EngineState s) {
  switch (s) {
    case EngineState::Boot: break;
    case EngineState::InitGL: break;
    case EngineState::Loading: break;
    case EngineState::Running: break;
    case EngineState::Paused: break;
    case EngineState::Suspended: break;
    case EngineState::ShuttingDown: /* destroy GL ctx/window */ break;
  }
}

void Engine::go(EngineState next) {
  if (state == next) return; 
  on_exit(state);
  state = next; 
  on_enter(state);
}

void Engine::process_events() {
  while(!events.empty()) {
    WindowEvent e = events.front(); events.pop(); // grap the first event, and remove 
    switch (e.type) {
      case WindowEvent::Close : running = false; break; 
      case WindowEvent::Resize :  width = e.a; height = e.b;  glViewport(0,0,width,height); break; 
      case WindowEvent::FocusLost : if (state == EngineState::Running) go(EngineState::Suspended); break; 
      case WindowEvent::FocusGained : if (state == EngineState::Suspended) go(EngineState::Running); break; 
      case WindowEvent::KeyDown : {
        if (e.a == 256 /*Esc*/) go(EngineState::ShuttingDown); 
        if (e.a == 'P') { 
          if (state == EngineState::Running) go(EngineState::Paused); 
          else if (state == EngineState::Paused) go(EngineState::Running);
        }

        break;
      }
      case WindowEvent::KeyUp : break;
    }
  }
}

void Engine::setup_input_callbacks() {
  glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
    auto* E = static_cast<Engine*>(glfwGetWindowUserPointer(win)); 
    
    if (!E || !E->captureMouse) return; 
    if (E->firstMouse) {E->lastMouseX = x; E->lastMouseY = y; E->firstMouse = false; return; }
    const float dx = static_cast<float>(x - E->lastMouseX);
    const float dy = static_cast<float>(y - E->lastMouseY);
    E->lastMouseX = x; E->lastMouseY = y; 

    E->camera.processMouse(dx, dy);

  });

  // Window Resize -> keep aspect current
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
    auto* E = static_cast<Engine*>(glfwGetWindowUserPointer(win));
    if (!E) return;
    E->width = w; E->height = h;  
    glViewport(0, 0, w, h);
    E->camera.aspect = (h > 0) ? (float)w / (float) h : E->camera.aspect;
  });
}

void Engine::update_fixed(double dt) {
  /* deterministic simulation steps */
  if (state != EngineState::Running) return;

  // ... simulate .. 

  angle += angular_velocity * static_cast<float>(dt);
  if (angle > 3.14159265f) angle -= 6.28318531f;
  if (angle < -3.14159265f) angle += 6.28318531f;
}

void Engine::update_variable(double dt) {
  if (state != EngineState::Running) return;

  // Handle WASD 
  const bool w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
  const bool s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
  const bool a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
  const bool d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

  // speed boost with Shift
  const bool boost = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  const float saved = camera.moveSpeed;
  if (boost) camera.moveSpeed *= 2.5f;

  camera.processKeyboard(w, s, a, d, static_cast<float>(dt));

  if (boost) camera.moveSpeed = saved;
}

void Engine::render() {
  if(!window) return; 
  glClearColor(0.1f, 0.12f, 0.2f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT); 

  if(state == EngineState::Running) {
    camera.aspect == static_cast<float> (width) / static_cast<float> (height);
    renderer.draw(angle, camera.getViewProj());
  }

  glfwSwapBuffers(window);
}