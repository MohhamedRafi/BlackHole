#include "engine.h"
#include <iostream>

bool Engine::on_enter(EngineState s) {
  switch (s) {
    case EngineState::Boot:      std::cout << "[enter] Boot\n"; return true;
    case EngineState::InitGL:    {
      std::cout << "[enter] InitGL\n"; /* create window, GL ctx */ return true;

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
      }

      glfwMakeContextCurrent(window);
      glfwSwapInterval(1); 


      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
          std::fprintf(stderr, "gladLoadGL failed\n");
          return false;
      }

      glViewport(0, 0, width, height);
      return true;
    };
    case EngineState::Loading:   std::cout << "[enter] Loading\n"; /* load assets */ return true;
    case EngineState::Running:   std::cout << "[enter] Running\n"; return true;
    case EngineState::Paused:    std::cout << "[enter] Paused\n"; return true;
    case EngineState::Suspended: std::cout << "[enter] Suspended\n"; return true;
    case EngineState::ShuttingDown: std::cout << "[enter] ShuttingDown\n"; return true;
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
      case WindowEvent::Resize : width = e.a; height = e.b; /* glViewport */ break;
      case WindowEvent::FocusLost : if (state == EngineState::Running) go(EngineState::Suspended);
      case WindowEvent::FocusGained : if (state == EngineState::Suspended) go(EngineState::Running); 
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

void Engine::update_fixed(double dt) {
  /* deterministic simulation steps */
  if (state != EngineState::Running) return;

  // ... simulate .. 
}

void Engine::update_variable(double dt) {

}

void Engine::render() {

}