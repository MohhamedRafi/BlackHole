#include "asset_loader.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

AssetLoader& AssetLoader::instance() {
  static AssetLoader L; 

  if (const char* env = std::getenv("ASSET_DIR")) {
    // override if we defined ASSET_DIR in env. 
    L.set_base_dir(env); 
  }

  return L;
}

void AssetLoader::set_base_dir(std::string base) {
  base_dir = std::move(base); 
}

std::string AssetLoader::resolve(const std::string& rel) const {
  if(base_dir.empty()) return rel; 
  if(base_dir.back() == '/') return base_dir + rel; 
  return base_dir + "/" + rel;
}

bool AssetLoader::read_text(const std::string& rel, std::string& out) const {
  const std::string path = resolve(rel);
  std::ifstream f(path, std::ios::in | std::ios::binary); 
  if (!f) return false; 
  std::ostringstream ss; ss << f.rdbuf(); // read buffer
  out = ss.str(); 
  return true;
}