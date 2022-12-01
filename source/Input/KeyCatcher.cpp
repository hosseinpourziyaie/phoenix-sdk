/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Key Catchers Handler
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-5
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <KeyCatcher.hpp>


namespace KeyCatcher
{
	std::vector<std::function<bool(int, bool)>> key_event_callbacks;
	std::vector<std::function<bool(int)>> char_event_callbacks;

	PlayerKeyState playerKeys;

	bool CL_CharEvent(int key)
	{
		bool should_intercept = false;
		for (auto& callback_func : char_event_callbacks)
			if (callback_func(key) == false) should_intercept = true;

		return should_intercept;
	}

	bool CL_KeyEvent(int key, bool down)
	{
		playerKeys.keys[key].down = down;

		if (down) {
			playerKeys.keys[key].repeats++;
			if (playerKeys.keys[key].repeats == 1) {
				playerKeys.anyKeyDown++;
			}
		}
		else {
			playerKeys.keys[key].repeats = 0;
			playerKeys.anyKeyDown--;
			if (playerKeys.anyKeyDown < 0) {
				playerKeys.anyKeyDown = 0;
			}
		}

		bool should_intercept = false;
		for (auto& callback_func : key_event_callbacks)
			if(callback_func(key, down) == false) should_intercept = true;

		return should_intercept;
	}

	bool IN_KeyCatcher(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool should_intercept = false;

		switch (uMsg)
		{
		case WM_KEYDOWN:
			should_intercept = CL_KeyEvent(static_cast<int>(wParam), 1);
			break;
		case WM_KEYUP:
			should_intercept = CL_KeyEvent(static_cast<int>(wParam), 0);
			break;
		case WM_CHAR:
			should_intercept = CL_CharEvent(static_cast<int>(wParam));
			break;
		case WM_MOUSEWHEEL:
			GET_WHEEL_DELTA_WPARAM((short)HIWORD(wParam)) ? should_intercept = CL_KeyEvent(K_MWHEELDOWN, 1) : should_intercept = CL_KeyEvent(K_MWHEELUP, 1);
			break;
		case WM_LBUTTONDOWN:
			should_intercept = CL_KeyEvent(K_MOUSE1, 1);
			break;
		case WM_LBUTTONUP:
			should_intercept = CL_KeyEvent(K_MOUSE1, 0);
			break;
		case WM_RBUTTONDOWN:
			should_intercept = CL_KeyEvent(K_MOUSE2, 1);
			break;
		case WM_RBUTTONUP:
			should_intercept = CL_KeyEvent(K_MOUSE2, 0);
			break;
		case WM_MBUTTONDOWN:
			should_intercept = CL_KeyEvent(K_MOUSE3, 1);
			break;
		case WM_MBUTTONUP:
			should_intercept = CL_KeyEvent(K_MOUSE3, 0);
			break;
		}

		return should_intercept;
	}
}