#pragma once

namespace utils::common
{
	void WriteString(const char* file, const char* szString);

	DWORD64 getModuleBaseAddr(LPCSTR Module = 0);
	DWORD64 getModuleImgSize(LPCSTR Module = 0);
	std::string getModuleFileName(LPCSTR Module = 0);

	std::string GetWindowTitleString(HWND hwnd);
	HWND getCurrentProcessWindowHandle();

	//const char* CatchMessage(const char* message, ...);
	float largestFloat(float array[], int length);

}
