// features.cpp
#include "features.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Features::Features(MemoryManager* memory, int screenWidth, int screenHeight)
    : m_memory(memory), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
    memset(&m_localPlayer, 0, sizeof(m_localPlayer));
    memset(&m_viewMatrix, 0, sizeof(m_viewMatrix));
}

bool Features::UpdateLocalPlayer() {
    if (!m_memory->IsAttached()) return false;

    uintptr_t base = m_memory->GetBaseAddress();
    
    // Resolve local player pointer (example chain)
    uintptr_t playerBase = m_memory->ResolvePointerChain(
        base + Offsets::LOCAL_PLAYER, { 0x0 }
    );
    
    if (playerBase == 0) return false;

    m_localPlayer.address = playerBase;
    m_localPlayer.health = m_memory->Read<float>(playerBase + Offsets::Player::HEALTH);
    m_localPlayer.maxHealth = m_memory->Read<float>(playerBase + Offsets::Player::MAX_HEALTH);
    m_localPlayer.position.x = m_memory->Read<float>(playerBase + Offsets::Player::POSITION_X);
    m_localPlayer.position.y = m_memory->Read<float>(playerBase + Offsets::Player::POSITION_Y);
    m_localPlayer.position.z = m_memory->Read<float>(playerBase + Offsets::Player::POSITION_Z);
    m_localPlayer.yaw = m_memory->Read<float>(playerBase + Offsets::Player::YAW);
    m_localPlayer.pitch = m_memory->Read<float>(playerBase + Offsets::Player::PITCH);
    m_localPlayer.teamId = m_memory->Read<int>(playerBase + Offsets::Player::TEAM_ID);

    return true;
}

bool Features::UpdateEntityList() {
    if (!m_memory->IsAttached()) return false;

    m_entities.clear();
    uintptr_t base = m_memory->GetBaseAddress();
    
    uintptr_t entityListBase = m_memory->ResolvePointerChain(
        base + Offsets::ENTITY_LIST, { 0x0 }
    );
    
    if (entityListBase == 0) return false;

    for (int i = 0; i < static_cast<int>(Offsets::EntityList::MAX_ENTITIES); i++) {
        uintptr_t entityAddr = entityListBase + (i * Offsets::EntityList::ENTITY_SIZE);
        
        // Check if entity exists (read a validity check)
        if (entityAddr == 0 || entityAddr == m_localPlayer.address) continue;
        
        GameEntity entity;
        entity.address = entityAddr;
        entity.health = m_memory->Read<float>(entityAddr + Offsets::Player::HEALTH);
        entity.maxHealth = m_memory->Read<float>(entityAddr + Offsets::Player::MAX_HEALTH);
        entity.isAlive = m_memory->Read<bool>(entityAddr + Offsets::Player::IS_ALIVE);
        
        if (!entity.isAlive || entity.health <= 0) continue;
        
        entity.position.x = m_memory->Read<float>(entityAddr + Offsets::Player::POSITION_X);
        entity.position.y = m_memory->Read<float>(entityAddr + Offsets::Player::POSITION_Y);
        entity.position.z = m_memory->Read<float>(entityAddr + Offsets::Player::POSITION_Z);
        entity.teamId = m_memory->Read<int>(entityAddr + Offsets::Player::TEAM_ID);
        entity.isVisible = m_memory->Read<bool>(entityAddr + Offsets::Player::VISIBILITY);
        entity.name = m_memory->ReadString(entityAddr + Offsets::Player::NAME, 32);
        entity.distance = m_localPlayer.position.Distance(entity.position);
        
        // World to screen projection
        entity.screenPos = WorldToScreen(entity.position);
        entity.headScreenPos = WorldToScreen(Vec3(entity.position.x, 
                                                    entity.position.y + 1.7f, 
                                                    entity.position.z));
        entity.onScreen = IsOnScreen(entity.screenPos);
        
        m_entities.push_back(entity);
    }

    // Sort by distance
    std::sort(m_entities.begin(), m_entities.end(),
        [](const GameEntity& a, const GameEntity& b) { return a.distance < b.distance; });

    return true;
}

bool Features::UpdateViewMatrix() {
    if (!m_memory->IsAttached()) return false;
    
    uintptr_t base = m_memory->GetBaseAddress();
    uintptr_t cameraBase = m_memory->ResolvePointerChain(
        base + Offsets::Camera::VIEW_MATRIX, { 0x0 }
    );
    
    if (cameraBase == 0) return false;
    
    m_memory->ReadBuffer(cameraBase, &m_viewMatrix, sizeof(Matrix4x4));
    return true;
}

Vec2 Features::WorldToScreen(const Vec3& worldPos) const {
    Vec2 screen;
    
    float w = m_viewMatrix.m[3][0] * worldPos.x + 
              m_viewMatrix.m[3][1] * worldPos.y + 
              m_viewMatrix.m[3][2] * worldPos.z + 
              m_viewMatrix.m[3][3];

    if (w < 0.001f) {
        return Vec2(-1, -1);
    }

    float invW = 1.0f / w;
    
    float x = m_viewMatrix.m[0][0] * worldPos.x + 
              m_viewMatrix.m[0][1] * worldPos.y + 
              m_viewMatrix.m[0][2] * worldPos.z + 
              m_viewMatrix.m[0][3];
    
    float y = m_viewMatrix.m[1][0] * worldPos.x + 
              m_viewMatrix.m[1][1] * worldPos.y + 
              m_viewMatrix.m[1][2] * worldPos.z + 
              m_viewMatrix.m[1][3];

    x *= invW;
    y *= invW;

    screen.x = (m_screenWidth / 2.0f) + (x * m_screenWidth / 2.0f);
    screen.y = (m_screenHeight / 2.0f) - (y * m_screenHeight / 2.0f);

    return screen;
}

bool Features::IsOnScreen(const Vec2& screenPos) const {
    return screenPos.x > 0 && screenPos.x < m_screenWidth && 
           screenPos.y > 0 && screenPos.y < m_screenHeight;
}

void Features::CalculateAimAngles(const Vec3& target, float& yaw, float& pitch) const {
    Vec3 delta = target - m_localPlayer.position;
    float distance = delta.Length();
    
    if (distance == 0) return;
    
    yaw = atan2f(delta.x, delta.z) * (180.0f / static_cast<float>(M_PI));
    pitch = -asinf(delta.y / distance) * (180.0f / static_cast<float>(M_PI));
}

float Features::GetAngleTo(const Vec3& target) const {
    Vec2 screenCenter(m_screenWidth / 2.0f, m_screenHeight / 2.0f);
    Vec2 targetScreen = WorldToScreen(target);
    
    if (!IsOnScreen(targetScreen)) return 9999.0f;
    
    Vec2 diff = targetScreen - screenCenter;
    return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

GameEntity* Features::FindBestTarget(const AimbotSettings& settings) {
    GameEntity* bestTarget = nullptr;
    float bestAngle = settings.fov;
    
    for (auto& entity : m_entities) {
        // Skip teammates
        if (entity.teamId == m_localPlayer.teamId) continue;
        
        // Skip dead entities
        if (!entity.isAlive || entity.health <= 0) continue;
        
        // Distance check
        if (entity.distance > settings.maxDistance) continue;
        
        // Visibility check
        if (settings.visibilityCheck && !entity.isVisible) continue;
        
        // FOV check
        float angle = GetAngleTo(entity.position);
        if (angle < bestAngle) {
            bestAngle = angle;
            bestTarget = &entity;
        }
    }
    
    return bestTarget;
}

void Features::Update() {
    UpdateViewMatrix();
    UpdateLocalPlayer();
    UpdateEntityList();
}

void Features::RunAimbot(const AimbotSettings& settings) {
    if (!settings.enabled) return;
    if (!(GetAsyncKeyState(settings.hotkey) & 0x8000)) return;
    
    GameEntity* target = FindBestTarget(settings);
    if (!target) return;
    
    // Calculate target position based on bone selection
    Vec3 targetPos = target->position;
    switch (settings.bone) {
        case 0: targetPos.y += 1.7f; break;  // Head
        case 1: targetPos.y += 1.2f; break;  // Chest
        case 2: targetPos.y += 0.8f; break;  // Pelvis
    }
    
    float targetYaw, targetPitch;
    CalculateAimAngles(targetPos, targetYaw, targetPitch);
    
    // Apply smoothing
    float currentYaw = m_localPlayer.yaw;
    float currentPitch = m_localPlayer.pitch;
    
    float smoothFactor = 1.0f / settings.smoothing;
    
    float newYaw = currentYaw + (targetYaw - currentYaw) * smoothFactor;
    float newPitch = currentPitch + (targetPitch - currentPitch) * smoothFactor;
    
    // Write new angles
    m_memory->Write<float>(m_localPlayer.address + Offsets::Player::YAW, newYaw);
    m_memory->Write<float>(m_localPlayer.address + Offsets::Player::PITCH, newPitch);
}

void Features::RunNoRecoil(bool enable) {
    if (!m_memory->IsAttached()) return;
    
    uintptr_t weaponBase = m_memory->Read<uintptr_t>(
        m_localPlayer.address + Offsets::Player::WEAPON_ID
    );
    if (weaponBase == 0) return;
    
    if (enable) {
        m_memory->Write<float>(weaponBase + Offsets::Weapon::RECOIL, 0.0f);
    }
}

void Features::RunNoSpread(bool enable) {
    if (!m_memory->IsAttached()) return;
    
    uintptr_t weaponBase = m_memory->Read<uintptr_t>(
        m_localPlayer.address + Offsets::Player::WEAPON_ID
    );
    if (weaponBase == 0) return;
    
    if (enable) {
        m_memory->Write<float>(weaponBase + Offsets::Weapon::SPREAD, 0.0f);
    }
}

void Features::RunRapidFire(bool enable) {
    if (!m_memory->IsAttached()) return;
    
    uintptr_t weaponBase = m_memory->Read<uintptr_t>(
        m_localPlayer.address + Offsets::Player::WEAPON_ID
    );
    if (weaponBase == 0) return;
    
    if (enable) {
        m_memory->Write<float>(weaponBase + Offsets::Weapon::FIRE_RATE, 0.01f);
    }
}

void Features::RunUnlimitedAmmo(bool enable) {
    if (!m_memory->IsAttached()) return;
    
    if (enable) {
        m_memory->Write<int>(m_localPlayer.address + Offsets::Player::AMMO, 999);
    }
}

void Features::RunSpeedHack(bool enable, float multiplier) {
    if (!m_memory->IsAttached() || !enable) return;
    // Speed modification would go here
    // This is game-engine specific
}

void Features::RunCustomFOV(bool enable, float fov) {
    if (!m_memory->IsAttached() || !enable) return;
    
    uintptr_t base = m_memory->GetBaseAddress();
    uintptr_t cameraBase = m_memory->ResolvePointerChain(
        base + Offsets::Camera::VIEW_MATRIX, { 0x0 }
    );
    
    if (cameraBase) {
        m_memory->Write<float>(cameraBase + Offsets::Camera::FOV, fov);
    }
}
