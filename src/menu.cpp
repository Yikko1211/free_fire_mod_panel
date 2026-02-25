// menu.cpp
#include "menu.h"
#include <string>
#include <sstream>
#include <iomanip>

Menu::Menu() : m_visible(true), m_activeTab(0) {}

Menu::~Menu() {
    Shutdown();
}

bool Menu::Initialize(IDirect3DDevice9* device, HWND window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    
    // Configurar plataforma IME (compatible con versiones nuevas de ImGui)
    ImGui::GetMainViewport()->PlatformHandleRaw = (void*)window;
    
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX9_Init(device);
    
    SetupStyle();
    
    return true;
}

void Menu::Shutdown() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Menu::SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // Dark theme with accent color
    colors[ImGuiCol_WindowBg]           = ImVec4(0.08f, 0.08f, 0.12f, 0.95f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.10f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.30f, 0.50f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.15f, 0.15f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.20f, 0.20f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.05f, 0.05f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.10f, 0.10f, 0.18f, 1.00f);
    colors[ImGuiCol_Header]             = ImVec4(0.20f, 0.20f, 0.35f, 0.50f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.30f, 0.30f, 0.50f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.35f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.25f, 0.50f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.30f, 0.35f, 0.65f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.35f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_CheckMark]          = ImVec4(0.40f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.30f, 0.50f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.40f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_Tab]                = ImVec4(0.15f, 0.15f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.30f, 0.35f, 0.60f, 1.00f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.25f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_Text]               = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    
    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;
    style.ChildRounding     = 4.0f;
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ScrollbarSize     = 12.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
}

void Menu::RenderAimbotTab() {
    ImGui::BeginChild("AimbotChild", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "AIMBOT SETTINGS");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Checkbox("Enable Aimbot", &m_aimbotSettings.enabled);
    
    if (m_aimbotSettings.enabled) {
        ImGui::Spacing();
        ImGui::SliderFloat("FOV Radius", &m_aimbotSettings.fov, 10.0f, 500.0f, "%.0f");
        ImGui::SliderFloat("Smoothing", &m_aimbotSettings.smoothing, 1.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Max Distance", &m_aimbotSettings.maxDistance, 50.0f, 1000.0f, "%.0f");
        
        ImGui::Spacing();
        ImGui::Checkbox("Visibility Check", &m_aimbotSettings.visibilityCheck);
        ImGui::Checkbox("Draw FOV Circle", &m_aimbotSettings.drawFOV);
        
        ImGui::Spacing();
        const char* bones[] = { "Head", "Chest", "Pelvis" };
        ImGui::Combo("Target Bone", &m_aimbotSettings.bone, bones, IM_ARRAYSIZE(bones));
        
        ImGui::Spacing();
        const char* hotkeys[] = { "Right Mouse", "Left Alt", "Left Shift", "Side Mouse 1", "Side Mouse 2" };
        int hotkeyIdx = 0;
        switch (m_aimbotSettings.hotkey) {
            case VK_RBUTTON: hotkeyIdx = 0; break;
            case VK_LMENU:   hotkeyIdx = 1; break;
            case VK_LSHIFT:  hotkeyIdx = 2; break;
            case VK_XBUTTON1: hotkeyIdx = 3; break;
            case VK_XBUTTON2: hotkeyIdx = 4; break;
        }
        if (ImGui::Combo("Hotkey", &hotkeyIdx, hotkeys, IM_ARRAYSIZE(hotkeys))) {
            int keys[] = { VK_RBUTTON, VK_LMENU, VK_LSHIFT, VK_XBUTTON1, VK_XBUTTON2 };
            m_aimbotSettings.hotkey = keys[hotkeyIdx];
        }
    }
    
    ImGui::EndChild();
}

void Menu::RenderESPTab() {
    ImGui::BeginChild("ESPChild", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "ESP SETTINGS");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Checkbox("Enable ESP", &m_espSettings.enabled);
    
    if (m_espSettings.enabled) {
        ImGui::Spacing();
        ImGui::Text("Draw Options:");
        ImGui::Checkbox("Bounding Box", &m_espSettings.drawBox);
        ImGui::Checkbox("Health Bar", &m_espSettings.drawHealth);
        ImGui::Checkbox("Player Name", &m_espSettings.drawName);
        ImGui::Checkbox("Distance", &m_espSettings.drawDistance);
        ImGui::Checkbox("Snap Lines", &m_espSettings.drawLine);
        ImGui::Checkbox("Skeleton", &m_espSettings.drawSkeleton);
        
        ImGui::Spacing();
        ImGui::Checkbox("Team Check", &m_espSettings.teamCheck);
        ImGui::SliderFloat("Max ESP Distance", &m_espSettings.maxDistance, 50.0f, 1000.0f, "%.0f");
        
        ImGui::Spacing();
        ImGui::Text("Colors:");
        ImGui::ColorEdit4("Enemy Color", m_espSettings.enemyColor);
        ImGui::ColorEdit4("Team Color", m_espSettings.teamColor);
        ImGui::ColorEdit4("Visible Color", m_espSettings.visibleColor);
    }
    
    ImGui::EndChild();
}

void Menu::RenderMiscTab() {
    ImGui::BeginChild("MiscChild", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "MISC SETTINGS");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Weapon Modifications:");
    ImGui::Checkbox("No Recoil", &m_miscSettings.noRecoil);
    ImGui::Checkbox("No Spread", &m_miscSettings.noSpread);
    ImGui::Checkbox("Rapid Fire", &m_miscSettings.rapidFire);
    ImGui::Checkbox("Unlimited Ammo", &m_miscSettings.unlimitedAmmo);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Movement:");
    ImGui::Checkbox("Speed Hack", &m_miscSettings.speedHack);
    if (m_miscSettings.speedHack) {
        ImGui::SliderFloat("Speed Multiplier", &m_miscSettings.speedMultiplier, 1.0f, 10.0f, "%.1fx");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Visual:");
    ImGui::Checkbox("No Fog", &m_miscSettings.noFog);
    ImGui::Checkbox("Custom FOV", &m_miscSettings.wideView);
    if (m_miscSettings.wideView) {
        ImGui::SliderFloat("FOV Value", &m_miscSettings.customFOV, 60.0f, 150.0f, "%.0f");
    }
    
    ImGui::EndChild();
}

void Menu::RenderInfoTab() {
    ImGui::BeginChild("InfoChild", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "INFORMATION");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
        "!! EDUCATIONAL PURPOSE ONLY !!");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "This mod menu is created purely for educational purposes "
        "to demonstrate game hacking concepts including:\n\n"
        "- Memory reading/writing via Windows API\n"
        "- Process attachment and manipulation\n"
        "- DirectX overlay rendering\n"
        "- World-to-screen projection\n"
        "- Pattern scanning\n"
        "- ImGui menu systems\n\n"
        "Using cheats in online games is:\n"
        "- Against Terms of Service\n"
        "- Can result in permanent bans\n"
        "- Ruins the experience for other players\n"
        "- May be illegal in some jurisdictions"
    );
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Hotkeys:");
    ImGui::BulletText("INSERT - Toggle Menu");
    ImGui::BulletText("END - Exit Application");
    ImGui::BulletText("F1 - Toggle ESP");
    ImGui::BulletText("F2 - Toggle Aimbot");
    
    ImGui::EndChild();
}

void Menu::RenderConfigTab() {
    ImGui::BeginChild("ConfigChild", ImVec2(0, 0), true);
    
    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "CONFIGURATION");
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Save Config", ImVec2(120, 30))) {
        // Config save logic
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Config", ImVec2(120, 30))) {
        // Config load logic
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Defaults", ImVec2(120, 30))) {
        m_aimbotSettings = AimbotSettings();
        m_espSettings = ESPSettings();
        m_miscSettings = MiscSettings();
    }
    
    ImGui::EndChild();
}

void Menu::RenderESPOverlay(Overlay* overlay, Features* features) {
    if (!m_espSettings.enabled) return;
    
    const auto& entities = features->GetEntities();
    const auto& localPlayer = features->GetLocalPlayer();
    
    for (const auto& entity : entities) {
        if (!entity.onScreen) continue;
        if (entity.distance > m_espSettings.maxDistance) continue;
        
        // Team check
        bool isTeammate = (entity.teamId == localPlayer.teamId);
        if (m_espSettings.teamCheck && isTeammate) continue;
        
        // Select color
        D3DCOLOR color;
        if (isTeammate) {
            color = D3DCOLOR_ARGB(
                static_cast<int>(m_espSettings.teamColor[3] * 255),
                static_cast<int>(m_espSettings.teamColor[0] * 255),
                static_cast<int>(m_espSettings.teamColor[1] * 255),
                static_cast<int>(m_espSettings.teamColor[2] * 255)
            );
        } else if (entity.isVisible) {
            color = D3DCOLOR_ARGB(
                static_cast<int>(m_espSettings.visibleColor[3] * 255),
                static_cast<int>(m_espSettings.visibleColor[0] * 255),
                static_cast<int>(m_espSettings.visibleColor[1] * 255),
                static_cast<int>(m_espSettings.visibleColor[2] * 255)
            );
        } else {
            color = D3DCOLOR_ARGB(
                static_cast<int>(m_espSettings.enemyColor[3] * 255),
                static_cast<int>(m_espSettings.enemyColor[0] * 255),
                static_cast<int>(m_espSettings.enemyColor[1] * 255),
                static_cast<int>(m_espSettings.enemyColor[2] * 255)
            );
        }
        
        // Calculate box dimensions
        float height = abs(entity.headScreenPos.y - entity.screenPos.y);
        float width = height * 0.6f;
        float boxX = entity.screenPos.x - width / 2;
        float boxY = entity.headScreenPos.y;
        
        // Draw bounding box
        if (m_espSettings.drawBox) {
            overlay->DrawCornerBox(boxX, boxY, width, height, color, 2.0f);
        }
        
        // Draw health bar
        if (m_espSettings.drawHealth) {
            overlay->DrawHealthBar(boxX - 6, boxY, 4, height, 
                                   entity.health, entity.maxHealth);
        }
        
        // Draw name
        if (m_espSettings.drawName && !entity.name.empty()) {
            overlay->DrawText(entity.name, entity.screenPos.x, boxY - 16, 
                            D3DCOLOR_ARGB(255, 255, 255, 255), true);
        }
        
        // Draw distance
        if (m_espSettings.drawDistance) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << entity.distance << "m";
            overlay->DrawText(ss.str(), entity.screenPos.x, 
                            entity.screenPos.y + 4, 
                            D3DCOLOR_ARGB(255, 200, 200, 200), true);
        }
        
        // Draw snap lines
        if (m_espSettings.drawLine) {
            overlay->DrawLine(
                static_cast<float>(overlay->GetWidth() / 2), 
                static_cast<float>(overlay->GetHeight()),
                entity.screenPos.x, entity.screenPos.y,
                color, 1.0f
            );
        }
    }
    
    // Draw aimbot FOV circle
    if (m_aimbotSettings.enabled && m_aimbotSettings.drawFOV) {
        overlay->DrawCircle(
            static_cast<float>(overlay->GetWidth() / 2),
            static_cast<float>(overlay->GetHeight() / 2),
            m_aimbotSettings.fov,
            D3DCOLOR_ARGB(100, 255, 255, 255),
            64
        );
    }
}

void Menu::Render(Overlay* overlay, Features* features) {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // Render ESP (always visible when enabled)
    RenderESPOverlay(overlay, features);
    
    // Render menu (toggleable)
    if (m_visible) {
        ImGui::SetNextWindowSize(ImVec2(500, 420), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Free Fire Mod Menu [EDUCATIONAL]", &m_visible, 
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
        
        // Header
        ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), 
            "FF Mod Menu v1.0 - Educational Purpose Only");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Tabs
        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Aimbot")) {
                RenderAimbotTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("ESP")) {
                RenderESPTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc")) {
                RenderMiscTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Config")) {
                RenderConfigTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Info")) {
                RenderInfoTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        
        // Footer
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
            "Press INSERT to toggle menu | END to exit");
        
        ImGui::End();
    }
    
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void Menu::ProcessInput() {
    // Toggle menu
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        Toggle();
    }
    
    // Quick toggles
    if (GetAsyncKeyState(VK_F1) & 1) {
        m_espSettings.enabled = !m_espSettings.enabled;
    }
    if (GetAsyncKeyState(VK_F2) & 1) {
        m_aimbotSettings.enabled = !m_aimbotSettings.enabled;
    }
}
