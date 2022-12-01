#pragma once

class InputHook final
{
public:
	static bool Initialize(HWND hWindow);
	static void Shutdown(HWND hWindow);

	typedef LRESULT(__stdcall* WndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT APIENTRY WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
