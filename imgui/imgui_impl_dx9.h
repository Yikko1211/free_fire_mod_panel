// imgui_impl_dx9.h
// ImGui DirectX9 Backend Implementation Header
// Based on dear imgui by Omar Cornut

#pragma once
#include "imgui.h"

struct IDirect3DDevice9;

IMGUI_IMPL_API bool ImGui_ImplDX9_Init(IDirect3DDevice9* device);
IMGUI_IMPL_API void ImGui_ImplDX9_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX9_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data);

// Called from message handler (on device lost/reset)
IMGUI_IMPL_API bool ImGui_ImplDX9_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplDX9_InvalidateDeviceObjects();
