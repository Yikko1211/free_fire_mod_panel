// offsets.h
// These are EXAMPLE/PLACEHOLDER offsets for educational demonstration
// Real offsets change with every game update

#pragma once

namespace Offsets {
    
    // ==========================================
    // NOTE: These are FAKE placeholder offsets
    // for educational demonstration only.
    // Real offsets require reverse engineering
    // each game update.
    // ==========================================
    
    // Base addresses (example placeholders)
    constexpr uintptr_t GLOBAL_CONTEXT       = 0x0;  // Placeholder
    constexpr uintptr_t GAME_WORLD           = 0x0;   
    constexpr uintptr_t LOCAL_PLAYER         = 0x0;   
    constexpr uintptr_t ENTITY_LIST          = 0x0;   
    
    // Player structure offsets
    namespace Player {
        constexpr uintptr_t HEALTH           = 0x100;
        constexpr uintptr_t MAX_HEALTH       = 0x104;
        constexpr uintptr_t POSITION_X       = 0x110;
        constexpr uintptr_t POSITION_Y       = 0x114;
        constexpr uintptr_t POSITION_Z       = 0x118;
        constexpr uintptr_t YAW              = 0x120;
        constexpr uintptr_t PITCH            = 0x124;
        constexpr uintptr_t TEAM_ID          = 0x130;
        constexpr uintptr_t IS_ALIVE         = 0x134;
        constexpr uintptr_t NAME             = 0x140;
        constexpr uintptr_t WEAPON_ID        = 0x200;
        constexpr uintptr_t AMMO             = 0x204;
        constexpr uintptr_t VISIBILITY       = 0x210;
    }

    // Camera offsets
    namespace Camera {
        constexpr uintptr_t VIEW_MATRIX      = 0x0;
        constexpr uintptr_t FOV              = 0x40;
        constexpr uintptr_t POSITION         = 0x44;
    }

    // Weapon offsets
    namespace Weapon {
        constexpr uintptr_t RECOIL           = 0x50;
        constexpr uintptr_t SPREAD           = 0x54;
        constexpr uintptr_t FIRE_RATE        = 0x58;
        constexpr uintptr_t BULLET_SPEED     = 0x5C;
        constexpr uintptr_t DAMAGE           = 0x60;
        constexpr uintptr_t RANGE            = 0x64;
    }

    // Entity list
    namespace EntityList {
        constexpr uintptr_t MAX_ENTITIES     = 50;
        constexpr uintptr_t ENTITY_SIZE      = 0x300;
        constexpr uintptr_t NEXT_ENTITY      = 0x8;
    }
}
