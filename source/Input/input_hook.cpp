/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Window Input Hook
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-3
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <input_hook.hpp>
#include <KeyCatcher.hpp>
#include <imgui_core.hpp>

InputHook::WndProc oWndProc = nullptr;
	
LRESULT APIENTRY InputHook::WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (KeyCatcher::IN_KeyCatcher(hWnd, uMsg, wParam, lParam) == true) return true;

	if (imgui::input_detour(hWnd, uMsg, wParam, lParam) == true) return true;


	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool InputHook::Initialize(HWND hWindow)
{
	log_debug_string("Installing Input Hook on windowHandle 0x%llX(%s)", hWindow, utils::common::GetWindowTitleString(hWindow).c_str());

	oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

	return true;
}

void InputHook::Shutdown(HWND hWindow)
{
	SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
}
