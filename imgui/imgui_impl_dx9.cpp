// imgui_impl_dx9.cpp
// ImGui DirectX9 Backend Implementation

#include "imgui.h"
#include "imgui_impl_dx9.h"

#include <d3d9.h>

// DirectX data
struct ImGui_ImplDX9_Data {
    IDirect3DDevice9*           pd3dDevice;
    IDirect3DVertexBuffer9*     pVB;
    IDirect3DIndexBuffer9*      pIB;
    IDirect3DTexture9*          FontTexture;
    int                         VertexBufferSize;
    int                         IndexBufferSize;

    ImGui_ImplDX9_Data() { memset((void*)this, 0, sizeof(*this)); VertexBufferSize = 5000; IndexBufferSize = 10000; }
};

struct CUSTOMVERTEX {
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_COL_TO_DX9_ARGB(_COL)  (_COL)
#else
#define IMGUI_COL_TO_DX9_ARGB(_COL)  (((_COL) & 0xFF00FF00) | (((_COL) >> 16) & 0xFF) | (((_COL) << 16) & 0xFF0000))
#endif

static ImGui_ImplDX9_Data* ImGui_ImplDX9_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplDX9_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

static void ImGui_ImplDX9_SetupRenderState(ImDrawData* draw_data) {
    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();

    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)draw_data->DisplaySize.x;
    vp.Height = (DWORD)draw_data->DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    bd->pd3dDevice->SetViewport(&vp);

    bd->pd3dDevice->SetPixelShader(nullptr);
    bd->pd3dDevice->SetVertexShader(nullptr);
    bd->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    bd->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    bd->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 
        D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | 
        D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    {
        float L = draw_data->DisplayPos.x + 0.5f;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
        float T = draw_data->DisplayPos.y + 0.5f;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 } } };
        D3DMATRIX mat_projection = { { {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f
        } } };
        bd->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

void ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data) {
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();
    IDirect3DDevice9* dev = bd->pd3dDevice;

    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount) {
        if (bd->pVB) { bd->pVB->Release(); bd->pVB = nullptr; }
        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (dev->CreateVertexBuffer(bd->VertexBufferSize * sizeof(CUSTOMVERTEX),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
            D3DPOOL_DEFAULT, &bd->pVB, nullptr) < 0)
            return;
    }
    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount) {
        if (bd->pIB) { bd->pIB->Release(); bd->pIB = nullptr; }
        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (dev->CreateIndexBuffer(bd->IndexBufferSize * sizeof(ImDrawIdx),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
            D3DPOOL_DEFAULT, &bd->pIB, nullptr) < 0)
            return;
    }

    // Backup DX9 state
    IDirect3DStateBlock9* d3d9_state_block = nullptr;
    if (dev->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;
    if (d3d9_state_block->Capture() < 0) {
        d3d9_state_block->Release();
        return;
    }

    D3DMATRIX last_world, last_view, last_projection;
    dev->GetTransform(D3DTS_WORLD, &last_world);
    dev->GetTransform(D3DTS_VIEW, &last_view);
    dev->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Copy vertices and indices
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx* idx_dst;
    if (bd->pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)),
        (void**)&vtx_dst, D3DLOCK_DISCARD) < 0) {
        d3d9_state_block->Release();
        return;
    }
    if (bd->pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)),
        (void**)&idx_dst, D3DLOCK_DISCARD) < 0) {
        bd->pVB->Unlock();
        d3d9_state_block->Release();
        return;
    }

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++) {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = IMGUI_COL_TO_DX9_ARGB(vtx_src->col);
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    bd->pVB->Unlock();
    bd->pIB->Unlock();

    dev->SetStreamSource(0, bd->pVB, 0, sizeof(CUSTOMVERTEX));
    dev->SetIndices(bd->pIB);
    dev->SetFVF(D3DFVF_CUSTOMVERTEX);

    ImGui_ImplDX9_SetupRenderState(draw_data);

    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr) {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplDX9_SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            } else {
                const RECT r = {
                    (LONG)(pcmd->ClipRect.x - clip_off.x),
                    (LONG)(pcmd->ClipRect.y - clip_off.y),
                    (LONG)(pcmd->ClipRect.z - clip_off.x),
                    (LONG)(pcmd->ClipRect.w - clip_off.y)
                };
                const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->GetTexID();
                dev->SetTexture(0, texture);
                dev->SetScissorRect(&r);
                dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset,
                    0, (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset,
                    pcmd->ElemCount / 3);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore state
    dev->SetTransform(D3DTS_WORLD, &last_world);
    dev->SetTransform(D3DTS_VIEW, &last_view);
    dev->SetTransform(D3DTS_PROJECTION, &last_projection);
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

bool ImGui_ImplDX9_Init(IDirect3DDevice9* device) {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized!");

    ImGui_ImplDX9_Data* bd = IM_NEW(ImGui_ImplDX9_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_dx9";

    bd->pd3dDevice = device;
    bd->pd3dDevice->AddRef();

    return true;
}

void ImGui_ImplDX9_Shutdown() {
    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();
    IM_ASSERT(bd != nullptr && "Not initialized or already shut down!");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplDX9_InvalidateDeviceObjects();
    if (bd->pd3dDevice) { bd->pd3dDevice->Release(); bd->pd3dDevice = nullptr; }
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    IM_DELETE(bd);
}

static bool ImGui_ImplDX9_CreateFontsTexture() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();

    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    bd->FontTexture = nullptr;
    if (bd->pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bd->FontTexture, nullptr) < 0)
        return false;

    D3DLOCKED_RECT tex_locked_rect;
    if (bd->FontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
        return false;

#ifndef IMGUI_USE_BGRA_PACKED_COLOR
    for (int y = 0; y < height; y++) {
        ImU32* pDst = (ImU32*)((unsigned char*)tex_locked_rect.pBits + (size_t)tex_locked_rect.Pitch * y);
        const ImU32* pSrc = (const ImU32*)(pixels + (size_t)width * bytes_per_pixel * y);
        for (int x = width; x > 0; x--, pDst++, pSrc++)
            *pDst = IMGUI_COL_TO_DX9_ARGB(*pSrc);
    }
#else
    for (int y = 0; y < height; y++) {
        memcpy((unsigned char*)tex_locked_rect.pBits + (size_t)tex_locked_rect.Pitch * y,
            pixels + (size_t)width * bytes_per_pixel * y, width * bytes_per_pixel);
    }
#endif

    bd->FontTexture->UnlockRect(0);

    io.Fonts->SetTexID((ImTextureID)bd->FontTexture);

    return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects() {
    if (!ImGui_ImplDX9_CreateFontsTexture())
        return false;
    return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects() {
    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();
    if (!bd || !bd->pd3dDevice) return;

    if (bd->pVB) { bd->pVB->Release(); bd->pVB = nullptr; }
    if (bd->pIB) { bd->pIB->Release(); bd->pIB = nullptr; }
    if (bd->FontTexture) {
        bd->FontTexture->Release();
        bd->FontTexture = nullptr;
        ImGui::GetIO().Fonts->SetTexID(nullptr);
    }
}

void ImGui_ImplDX9_NewFrame() {
    ImGui_ImplDX9_Data* bd = ImGui_ImplDX9_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplDX9_Init()?");

    if (!bd->FontTexture)
        ImGui_ImplDX9_CreateDeviceObjects();
}
