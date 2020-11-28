// dear imgui: Renderer for DirectX9
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once

#include "imgui.h"
#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9")

struct IDirect3DDevice9;
struct IDirect3DTexture9;

IMGUI_IMPL_API bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
IMGUI_IMPL_API bool LoadTextureFromFile(const wchar_t* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);

IMGUI_IMPL_API bool     ImGui_ImplDX9_Init(IDirect3DDevice9* device);
IMGUI_IMPL_API void     ImGui_ImplDX9_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX9_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API void     ImGui_ImplDX9_DestroyFontsTexture();

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_IMPL_API bool     ImGui_ImplDX9_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX9_InvalidateDeviceObjects();

IMGUI_IMPL_API ImTextureID    ImGui_CreateTextureRGBA(int width, int height, const unsigned char* data);
IMGUI_IMPL_API void           ImGui_DestroyTexture(ImTextureID texture);
