// overlay.cpp
#include "overlay.h"
#include <dwmapi.h>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Overlay::Overlay() 
    : m_overlayWindow(nullptr), m_targetWindow(nullptr),
      m_d3d(nullptr), m_device(nullptr), m_font(nullptr), m_line(nullptr),
      m_width(0), m_height(0), m_initialized(false) {
    memset(&m_windowClass, 0, sizeof(m_windowClass));
    memset(&m_params, 0, sizeof(m_params));
}

Overlay::~Overlay() {
    Shutdown();
}

bool Overlay::CreateOverlayWindow() {
    m_windowClass.cbSize = sizeof(WNDCLASSEX);
    m_windowClass.style = CS_HREDRAW | CS_VREDRAW;
    m_windowClass.lpfnWndProc = OverlayWndProc;
    m_windowClass.hInstance = GetModuleHandle(nullptr);
    m_windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_windowClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    m_windowClass.lpszClassName = "OverlayClass";
    
    RegisterClassEx(&m_windowClass);

    m_targetWindow = FindWindowA(nullptr, m_targetWindowName.c_str());
    if (!m_targetWindow) {
        std::cout << "[Overlay] Target window not found: " << m_targetWindowName << std::endl;
        return false;
    }

    RECT targetRect;
    GetWindowRect(m_targetWindow, &targetRect);
    m_width = targetRect.right - targetRect.left;
    m_height = targetRect.bottom - targetRect.top;

    m_overlayWindow = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        "OverlayClass",
        "Overlay",
        WS_POPUP,
        targetRect.left, targetRect.top,
        m_width, m_height,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!m_overlayWindow) {
        std::cout << "[Overlay] Failed to create overlay window." << std::endl;
        return false;
    }

    // Make window transparent
    SetLayeredWindowAttributes(m_overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);
    
    // Extend frame into client area for glass effect
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_overlayWindow, &margins);

    ShowWindow(m_overlayWindow, SW_SHOW);
    UpdateWindow(m_overlayWindow);

    std::cout << "[Overlay] Window created: " << m_width << "x" << m_height << std::endl;
    return true;
}

bool Overlay::InitDirectX() {
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d) {
        std::cout << "[Overlay] Failed to create D3D9." << std::endl;
        return false;
    }

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
        std::cout << "[Overlay] Failed to create D3D9 device." << std::endl;
        return false;
    }

    D3DXCreateFontA(m_device, 14, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                    "Arial", &m_font);

    D3DXCreateLine(m_device, &m_line);

    std::cout << "[Overlay] DirectX initialized." << std::endl;
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
    if (m_font) { m_font->Release(); m_font = nullptr; }
    if (m_line) { m_line->Release(); m_line = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }
    if (m_d3d) { m_d3d->Release(); m_d3d = nullptr; }
    if (m_overlayWindow) { DestroyWindow(m_overlayWindow); m_overlayWindow = nullptr; }
    
    UnregisterClassA("OverlayClass", GetModuleHandle(nullptr));
    m_initialized = false;
}

void Overlay::UpdateWindowPosition() {
    if (!m_targetWindow) return;
    
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
    m_device->BeginScene();
    
    return true;
}

void Overlay::EndFrame() {
    if (!m_device) return;
    m_device->EndScene();
    m_device->Present(nullptr, nullptr, nullptr, nullptr);
}

void Overlay::DrawText(const std::string& text, float x, float y, 
                       D3DCOLOR color, bool centered) {
    if (!m_font) return;
    
    RECT rect;
    if (centered) {
        SetRect(&rect, static_cast<int>(x) - 200, static_cast<int>(y), 
                static_cast<int>(x) + 200, static_cast<int>(y) + 20);
        m_font->DrawTextA(nullptr, text.c_str(), -1, &rect, 
                         DT_CENTER | DT_NOCLIP, color);
    } else {
        SetRect(&rect, static_cast<int>(x), static_cast<int>(y), 
                static_cast<int>(x) + 500, static_cast<int>(y) + 20);
        m_font->DrawTextA(nullptr, text.c_str(), -1, &rect, 
                         DT_LEFT | DT_NOCLIP, color);
    }
}

void Overlay::DrawLine(float x1, float y1, float x2, float y2, 
                       D3DCOLOR color, float thickness) {
    if (!m_line) return;
    
    D3DXVECTOR2 points[2] = {
        D3DXVECTOR2(x1, y1),
        D3DXVECTOR2(x2, y2)
    };
    
    m_line->SetWidth(thickness);
    m_line->Begin();
    m_line->Draw(points, 2, color);
    m_line->End();
}

void Overlay::DrawRect(float x, float y, float w, float h, 
                       D3DCOLOR color, float thickness) {
    DrawLine(x, y, x + w, y, color, thickness);
    DrawLine(x, y, x, y + h, color, thickness);
    DrawLine(x + w, y, x + w, y + h, color, thickness);
    DrawLine(x, y + h, x + w, y + h, color, thickness);
}

void Overlay::DrawFilledRect(float x, float y, float w, float h, D3DCOLOR color) {
    struct Vertex {
        float x, y, z, rhw;
        DWORD color;
    };
    
    Vertex vertices[] = {
        { x,     y,     0, 1, color },
        { x + w, y,     0, 1, color },
        { x,     y + h, 0, 1, color },
        { x + w, y + h, 0, 1, color }
    };
    
    m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    m_device->SetTexture(0, nullptr);
    m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
}

void Overlay::DrawCircle(float x, float y, float radius, D3DCOLOR color, int segments) {
    if (!m_line) return;
    
    std::vector<D3DXVECTOR2> points(segments + 1);
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * D3DX_PI * i) / segments;
        points[i] = D3DXVECTOR2(
            x + radius * cosf(angle),
            y + radius * sinf(angle)
        );
    }
    
    m_line->SetWidth(1.0f);
    m_line->Begin();
    m_line->Draw(points.data(), static_cast<DWORD>(points.size()), color);
    m_line->End();
}

void Overlay::DrawHealthBar(float x, float y, float w, float h, 
                             float health, float maxHealth) {
    float healthPercent = health / maxHealth;
    if (healthPercent < 0.0f) healthPercent = 0.0f;
    if (healthPercent > 1.0f) healthPercent = 1.0f;
    
    // Background
    DrawFilledRect(x - 1, y - 1, w + 2, h + 2, D3DCOLOR_ARGB(200, 0, 0, 0));
    
    // Health bar color (green -> yellow -> red)
    D3DCOLOR barColor;
    if (healthPercent > 0.5f) {
        barColor = D3DCOLOR_ARGB(255, 
            static_cast<int>((1.0f - healthPercent) * 2 * 255), 255, 0);
    } else {
        barColor = D3DCOLOR_ARGB(255, 255, 
            static_cast<int>(healthPercent * 2 * 255), 0);
    }
    
    DrawFilledRect(x, y, w * healthPercent, h, barColor);
}

void Overlay::DrawCornerBox(float x, float y, float w, float h, 
                             D3DCOLOR color, float thickness) {
    float cornerLength = w * 0.25f;
    
    // Top-left
    DrawLine(x, y, x + cornerLength, y, color, thickness);
    DrawLine(x, y, x, y + cornerLength, color, thickness);
    
    // Top-right
    DrawLine(x + w, y, x + w - cornerLength, y, color, thickness);
    DrawLine(x + w, y, x + w, y + cornerLength, color, thickness);
    
    // Bottom-left
    DrawLine(x, y + h, x + cornerLength, y + h, color, thickness);
    DrawLine(x, y + h, x, y + h - cornerLength, color, thickness);
    
    // Bottom-right
    DrawLine(x + w, y + h, x + w - cornerLength, y + h, color, thickness);
    DrawLine(x + w, y + h, x + w, y + h - cornerLength, color, thickness);
}

void Overlay::CleanupDirectX() {
    if (m_font) { m_font->Release(); m_font = nullptr; }
    if (m_line) { m_line->Release(); m_line = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }
    if (m_d3d) { m_d3d->Release(); m_d3d = nullptr; }
}
