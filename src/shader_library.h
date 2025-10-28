#pragma once
#include "shader.h"
#include "asset_loader.h"

#include <unordered_map>
#include <string>

struct ShaderLibrary {
  std::unordered_map<std::string, ShaderProgram> progs; 
  void shutdown(); 

  const ShaderProgram& get_flat_color();

  
  const ShaderProgram& get_from_files(const std::string& name, const std::string& vs_rel, const std::string& fs_rel);

};