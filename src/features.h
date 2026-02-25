// features.h
#pragma once

#include "memory.h"
#include "offsets.h"
#include <vector>
#include <cmath>

// Game entity structure
struct GameEntity {
    uintptr_t address;
    Vec3 position;
    float health;
    float maxHealth;
    int teamId;
    bool isAlive;
    bool isVisible;
    std::string name;
    float distance;     // from local player
    Vec2 screenPos;     // projected screen position
    Vec2 headScreenPos;
    bool onScreen;
};

struct LocalPlayer {
    uintptr_t address;
    Vec3 position;
    float health;
    float maxHealth;
    int teamId;
    float yaw;
    float pitch;
};

// Feature settings
struct AimbotSettings {
    bool enabled = false;
    float fov = 90.0f;           // FOV circle radius
    float smoothing = 5.0f;       // Aim smoothing (higher = slower)
    bool visibilityCheck = true;
    int bone = 0;                 // 0 = head, 1 = chest, 2 = pelvis
    int hotkey = VK_RBUTTON;
    bool drawFOV = true;
    float maxDistance = 300.0f;
};

struct ESPSettings {
    bool enabled = false;
    bool drawBox = true;
    bool drawHealth = true;
    bool drawName = true;
    bool drawDistance = true;
    bool drawLine = false;
    bool drawSkeleton = false;
    bool teamCheck = true;
    float maxDistance = 500.0f;
    
    // Colors (RGBA)
    float enemyColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float teamColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float visibleColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
};

struct MiscSettings {
    bool noRecoil = false;
    bool noSpread = false;
    bool rapidFire = false;
    bool unlimitedAmmo = false;
    bool speedHack = false;
    float speedMultiplier = 2.0f;
    bool flyHack = false;
    bool noFog = false;
    bool wideView = false;
    float customFOV = 90.0f;
};

class Features {
private:
    MemoryManager* m_memory;
    LocalPlayer m_localPlayer;
    std::vector<GameEntity> m_entities;
    Matrix4x4 m_viewMatrix;
    
    int m_screenWidth;
    int m_screenHeight;

    // Internal functions
    bool UpdateLocalPlayer();
    bool UpdateEntityList();
    bool UpdateViewMatrix();
    
    Vec2 WorldToScreen(const Vec3& worldPos) const;
    bool IsOnScreen(const Vec2& screenPos) const;
    
    float GetAngleTo(const Vec3& target) const;
    void CalculateAimAngles(const Vec3& target, float& yaw, float& pitch) const;
    
    GameEntity* FindBestTarget(const AimbotSettings& settings);

public:
    Features(MemoryManager* memory, int screenWidth, int screenHeight);
    ~Features() = default;

    // Update game state
    void Update();

    // Feature implementations
    void RunAimbot(const AimbotSettings& settings);
    void RunESP(const ESPSettings& settings);  // Returns entities for rendering
    void RunNoRecoil(bool enable);
    void RunNoSpread(bool enable);
    void RunRapidFire(bool enable);
    void RunUnlimitedAmmo(bool enable);
    void RunSpeedHack(bool enable, float multiplier);
    void RunCustomFOV(bool enable, float fov);

    // Getters
    const LocalPlayer& GetLocalPlayer() const { return m_localPlayer; }
    const std::vector<GameEntity>& GetEntities() const { return m_entities; }
    const Matrix4x4& GetViewMatrix() const { return m_viewMatrix; }
};
