// menu.h
#pragma once

#include "features.h"
#include "overlay.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

class Menu {
private:
    bool m_visible;
    int m_activeTab;
    
    // Feature settings
    AimbotSettings m_aimbotSettings;
    ESPSettings m_espSettings;
    MiscSettings m_miscSettings;
    
    // Style
    void SetupStyle();
    
    // Tab rendering
    void RenderAimbotTab();
    void RenderESPTab();
    void RenderMiscTab();
    void RenderInfoTab();
    void RenderConfigTab();
    
    // ESP Rendering (on overlay)
    void RenderESPOverlay(Overlay* overlay, Features* features);

public:
    Menu();
    ~Menu();
    
    bool Initialize(IDirect3DDevice9* device, HWND window);
    void Shutdown();
    
    void Toggle() { m_visible = !m_visible; }
    bool IsVisible() const { return m_visible; }
    
    void Render(Overlay* overlay, Features* features);
    void ProcessInput();
    
    // Getters for settings
    AimbotSettings& GetAimbotSettings() { return m_aimbotSettings; }
    ESPSettings& GetESPSettings() { return m_espSettings; }
    MiscSettings& GetMiscSettings() { return m_miscSettings; }
};
