// config.cpp
#include "config.h"
#include <iostream>
#include <sstream>

bool Config::Save(const std::string& filename,
                  const AimbotSettings& aimbot,
                  const ESPSettings& esp,
                  const MiscSettings& misc) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    
    // Simple binary serialization
    file.write(reinterpret_cast<const char*>(&aimbot), sizeof(AimbotSettings));
    file.write(reinterpret_cast<const char*>(&esp), sizeof(ESPSettings));
    file.write(reinterpret_cast<const char*>(&misc), sizeof(MiscSettings));
    
    file.close();
    std::cout << "[Config] Saved to " << filename << std::endl;
    return true;
}

bool Config::Load(const std::string& filename,
                  AimbotSettings& aimbot,
                  ESPSettings& esp,
                  MiscSettings& misc) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    
    file.read(reinterpret_cast<char*>(&aimbot), sizeof(AimbotSettings));
    file.read(reinterpret_cast<char*>(&esp), sizeof(ESPSettings));
    file.read(reinterpret_cast<char*>(&misc), sizeof(MiscSettings));
    
    file.close();
    std::cout << "[Config] Loaded from " << filename << std::endl;
    return true;
}
