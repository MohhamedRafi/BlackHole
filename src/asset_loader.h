#pragma once
#include <string> 
#include <vector>
#include <optional> 

class AssetLoader {
  public: 
    static AssetLoader& instance(); // singleton 


    void        set_base_dir(std::string base); 
    std::string resolve(const std::string& rel) const; 
    bool        read_text(const std::string& rel, std::string& out) const; 

  private:
    std::string base_dir = "assets";
};