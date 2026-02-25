#include "overlay.h"
#include <dwmapi.h>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")

// Forward declare del handler de ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Variable global para acceder al overlay desde WndProc
static Overlay* g_overlay = nullptr;

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Pasar mensajes a ImGui PRIMERO
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        return 0;

    // Prevenir que la ventana pierda el foco cuando está interactiva
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        return 0;

    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Overlay::Overlay()
    : m_overlayWindow(nullptr), m_targetWindow(nullptr),
    m_d3d(nullptr), m_device(nullptr),
    m_width(0), m_height(0), m_initialized(false), m_interactable(true) {
    memset(&m_windowClass, 0, sizeof(m_windowClass));
    memset(&m_params, 0, sizeof(m_params));
    g_overlay = this;
}

Overlay::~Overlay() {
    Shutdown();
    g_overlay = nullptr;
}

bool Overlay::CreateOverlayWindow() {
    m_windowClass.cbSize = sizeof(WNDCLASSEX);
    m_windowClass.style = CS_HREDRAW | CS_VREDRAW;
    m_windowClass.lpfnWndProc = OverlayWndProc;
    m_windowClass.hInstance = GetModuleHandle(nullptr);
    m_windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_windowClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    m_windowClass.lpszClassName = "FFOverlayClass";

    RegisterClassEx(&m_windowClass);

    m_targetWindow = FindWindowA(nullptr, m_targetWindowName.c_str());
    if (!m_targetWindow) {
        std::cout << "[Overlay] Ventana objetivo no encontrada: " << m_targetWindowName << std::endl;
        return false;
    }

    RECT targetRect;
    GetWindowRect(m_targetWindow, &targetRect);
    m_width = targetRect.right - targetRect.left;
    m_height = targetRect.bottom - targetRect.top;

    // Crear SIN WS_EX_TRANSPARENT para poder capturar clicks
    m_overlayWindow = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        "FFOverlayClass",
        "FF Overlay",
        WS_POPUP,
        targetRect.left, targetRect.top,
        m_width, m_height,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_overlayWindow) {
        std::cout << "[Overlay] Error al crear ventana overlay." << std::endl;
        return false;
    }

    // Hacer el fondo transparente (color negro = transparente)
    SetLayeredWindowAttributes(m_overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Extender el frame para efecto de cristal
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_overlayWindow, &margins);

    ShowWindow(m_overlayWindow, SW_SHOWNOACTIVATE);
    UpdateWindow(m_overlayWindow);

    // Por defecto, la ventana es interactiva (menú visible)
    m_interactable = true;

    std::cout << "[Overlay] Ventana creada: " << m_width << "x" << m_height << std::endl;
    return true;
}

bool Overlay::InitDirectX() {
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d) {
        std::cout << "[Overlay] Error al crear D3D9." << std::endl;
        return false;
    }

    ZeroMemory(&m_params, sizeof(m_params));
    m_params.Windowed = TRUE;
    m_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_params.hDeviceWindow = m_overlayWindow;
    m_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    m_params.BackBufferFormat = D3DFMT_A8R8G8B8;
    m_params.BackBufferWidth = m_width;
    m_params.BackBufferHeight = m_height;
    m_params.EnableAutoDepthStencil = TRUE;
    m_params.AutoDepthStencilFormat = D3DFMT_D16;
    m_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = m_d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_overlayWindow,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &m_params, &m_device
    );

    if (FAILED(hr)) {
        // Intentar con software vertex processing
        hr = m_d3d->CreateDevice(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_overlayWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &m_params, &m_device
        );
        if (FAILED(hr)) {
            std::cout << "[Overlay] Error al crear dispositivo D3D9. HRESULT: " << hr << std::endl;
            return false;
        }
    }

    std::cout << "[Overlay] DirectX inicializado." << std::endl;
    return true;
}

bool Overlay::Initialize(const std::string& targetWindowName) {
    m_targetWindowName = targetWindowName;

    if (!CreateOverlayWindow()) return false;
    if (!InitDirectX()) return false;

    m_initialized = true;
    return true;
}

void Overlay::Shutdown() {
    CleanupDirectX();
    if (m_overlayWindow) { DestroyWindow(m_overlayWindow); m_overlayWindow = nullptr; }
    UnregisterClassA("FFOverlayClass", GetModuleHandle(nullptr));
    m_initialized = false;
}

void Overlay::CleanupDirectX() {
    if (m_device) { m_device->Release(); m_device = nullptr; }
    if (m_d3d) { m_d3d->Release(); m_d3d = nullptr; }
}

void Overlay::SetInteractable(bool interactable) {
    if (m_interactable == interactable) return;
    m_interactable = interactable;

    LONG_PTR style = GetWindowLongPtr(m_overlayWindow, GWL_EXSTYLE);

    if (interactable) {
        // QUITAR WS_EX_TRANSPARENT = la ventana CAPTURA clicks
        style &= ~WS_EX_TRANSPARENT;
        SetWindowLongPtr(m_overlayWindow, GWL_EXSTYLE, style);
        std::cout << "[Overlay] Modo interactivo: ON (clicks van al menu)" << std::endl;
    }
    else {
        // AGREGAR WS_EX_TRANSPARENT = los clicks PASAN AL JUEGO
        style |= WS_EX_TRANSPARENT;
        SetWindowLongPtr(m_overlayWindow, GWL_EXSTYLE, style);
        std::cout << "[Overlay] Modo interactivo: OFF (clicks van al juego)" << std::endl;
    }
}

void Overlay::UpdateWindowPosition() {
    if (!m_targetWindow || !IsWindow(m_targetWindow)) return;

    RECT targetRect;
    GetWindowRect(m_targetWindow, &targetRect);

    int newWidth = targetRect.right - targetRect.left;
    int newHeight = targetRect.bottom - targetRect.top;

    MoveWindow(m_overlayWindow, targetRect.left, targetRect.top,
        newWidth, newHeight, TRUE);

    m_width = newWidth;
    m_height = newHeight;
}

bool Overlay::BeginFrame() {
    if (!m_device) return false;

    m_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    if (FAILED(m_device->BeginScene())) return false;

    return true;
}

void Overlay::EndFrame() {
    if (!m_device) return;
    m_device->EndScene();

    HRESULT hr = m_device->Present(nullptr, nullptr, nullptr, nullptr);

    // Handle device lost
    if (hr == D3DERR_DEVICELOST) {
        if (m_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            m_device->Reset(&m_params);
        }
    }
}