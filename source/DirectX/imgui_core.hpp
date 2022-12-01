#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace imgui
{
	extern bool override_controls;

	extern ImFont* m_pFont;
	extern ImFont* phoenix_pFont;

	void add_menu(const std::function<void()> callback);
	bool input_detour(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void draw_rect(int x, int y, int w, int h, float* color);
	void draw_text(std::string text, float x, float y, float* color, ImFont* font = ImGui::GetFont(), float fontSize = ImGui::GetFontSize());
}
