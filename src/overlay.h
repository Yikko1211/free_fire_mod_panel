#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <string>

#pragma comment(lib, "d3d9.lib")

class Overlay {
private:
    HWND m_overlayWindow;
    HWND m_targetWindow;
    WNDCLASSEX m_windowClass;

    IDirect3D9* m_d3d;
    IDirect3DDevice9* m_device;
    D3DPRESENT_PARAMETERS m_params;

    int m_width;
    int m_height;
    bool m_initialized;
    bool m_interactable;

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

    // NUEVO: Controlar si la ventana captura clicks o no
    void SetInteractable(bool interactable);
    bool IsInteractable() const { return m_interactable; }

    // Getters
    IDirect3DDevice9* GetDevice() const { return m_device; }
    HWND GetWindow() const { return m_overlayWindow; }
    HWND GetTargetWindow() const { return m_targetWindow; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    bool IsInitialized() const { return m_initialized; }
};
