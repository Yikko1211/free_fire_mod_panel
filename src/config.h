// config.h
#pragma once

#include "features.h"
#include <string>
#include <fstream>

class Config {
public:
    static bool Save(const std::string& filename, 
                     const AimbotSettings& aimbot,
                     const ESPSettings& esp, 
                     const MiscSettings& misc);
    
    static bool Load(const std::string& filename,
                     AimbotSettings& aimbot,
                     ESPSettings& esp,
                     MiscSettings& misc);
};
