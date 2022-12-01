#pragma once

namespace KeyCatcher
{
	extern std::vector<std::function<bool(int, bool)>> key_event_callbacks;
	extern std::vector<std::function<bool(int)>> char_event_callbacks;

	enum keyNum_t
	{
		K_NONE = 0x00,
		K_TAB = 0x09,
		K_ENTER = 0x0D,
		K_ESCAPE = 0x1B,
		K_SPACE = 0x20,
		K_GRAVE = 0xC0,
		K_TILDE = 0xC0,
		K_BACKSPACE = 0x08,
		K_LEFTARROW = 0x25,
		K_UPARROW = 0x26,
		K_RIGHTARROW = 0x27,
		K_DOWNARROW = 0x28,
		K_ALT = 0x9E,
		K_CTRL = 0x11,
		K_SHIFT = 0x10,
		K_INS = 0x2D,
		K_DEL = 0x2E,
		K_PGDN = 0x22,
		K_PGUP = 0x21,
		K_HOME = 0x24,
		K_END = 0x23,
		K_MOUSE1 = 0xC8,
		K_MOUSE2 = 0xC9,
		K_MOUSE3 = 0xCA,
		K_MOUSE4 = 0xCB,
		K_MOUSE5 = 0xCC,
		K_MWHEELDOWN = 0xCD,
		K_MWHEELUP = 0xCE
	};

	struct KeyState
	{
		int down;
		int repeats;
		int binding;
	};

	struct PlayerKeyState
	{
		int overstrikeMode;
		int anyKeyDown;
		KeyState keys[256];
	};

	extern PlayerKeyState playerKeys;


	bool IN_KeyCatcher(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}