// imgui_impl_win32.h
// ImGui Win32 Platform Backend Header

#pragma once
#include "imgui.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

IMGUI_IMPL_API bool ImGui_ImplWin32_Init(HWND hWnd);
IMGUI_IMPL_API void ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void ImGui_ImplWin32_NewFrame();

// Win32 message handler - call from your WndProc
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Enable DPI awareness
IMGUI_IMPL_API void ImGui_ImplWin32_EnableDpiAwareness();
IMGUI_IMPL_API float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);
