// overlay.h
#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <functional>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

class Overlay {
private:
    HWND m_overlayWindow;
    HWND m_targetWindow;
    WNDCLASSEX m_windowClass;
    
    IDirect3D9* m_d3d;
    IDirect3DDevice9* m_device;
    D3DPRESENT_PARAMETERS m_params;
    
    ID3DXFont* m_font;
    ID3DXLine* m_line;
    
    int m_width;
    int m_height;
    bool m_initialized;
    
    std::string m_targetWindowName;

    bool CreateOverlayWindow();
    bool InitDirectX();
    void CleanupDirectX();

public:
    Overlay();
    ~Overlay();

    bool Initialize(const std::string& targetWindowName);
    void Shutdown();
    
    bool BeginFrame();
    void EndFrame();
    
    void UpdateWindowPosition();
    
    // Drawing functions
    void DrawText(const std::string& text, float x, float y, 
                  D3DCOLOR color, bool centered = false);
    void DrawLine(float x1, float y1, float x2, float y2, 
                  D3DCOLOR color, float thickness = 1.0f);
    void DrawRect(float x, float y, float w, float h, 
                  D3DCOLOR color, float thickness = 1.0f);
    void DrawFilledRect(float x, float y, float w, float h, D3DCOLOR color);
    void DrawCircle(float x, float y, float radius, 
                    D3DCOLOR color, int segments = 32);
    void DrawHealthBar(float x, float y, float w, float h, 
                       float health, float maxHealth);
    void DrawCornerBox(float x, float y, float w, float h, 
                       D3DCOLOR color, float thickness = 1.0f);
    
    // Getters
    IDirect3DDevice9* GetDevice() const { return m_device; }
    HWND GetWindow() const { return m_overlayWindow; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    bool IsInitialized() const { return m_initialized; }
};
