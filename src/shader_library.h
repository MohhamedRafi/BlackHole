#pragma once
#include "shader.h"
#include <unordered_map>
#include <string>

struct ShaderLibrary {
  std::unordered_map<std::string, ShaderProgram> progs; 
  void shutdown(); 

  const ShaderProgram& get_flat_color(); // example
};