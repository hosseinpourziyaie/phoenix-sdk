/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Main Entry
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-28
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <dx11_hook.hpp>
#include <dx12_hook.hpp>

namespace
{
	void ShutdownListenerLoop()
	{
		do {
			Sleep(100);
		} while (!(GetAsyncKeyState(VK_DELETE) & 0x1));
	}

	void SpawnConsole()
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 6031)
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
#pragma warning(pop)
	}

	bool Initialize_Graphical_Overlay_Hook()
	{
		if (GetModuleHandle("d3d12.dll") != NULL) {
			log_error_string("Directx12 API is not Supported yet");
			return false;
		}
		else if (GetModuleHandle("d3d11.dll") != NULL) {
			log_debug_string("Found Directx11 API Module(d3d11.dll)");
			return DirectX11::Initialize();
		}
		else {
			log_error_string("Couldnt find supported graphical API");
			return false;
		}
	}

	void Shutdown_Graphical_Overlay_Hook()
	{
		if (GetModuleHandle("d3d12.dll") != NULL) {
			// UN-IMPLEMENTED
		}
		else if (GetModuleHandle("d3d11.dll") != NULL) {
			DirectX11::Shutdown();
		}
		else {
			// UN-IMPLEMENTED
		}
	}

	DWORD WINAPI MainThread(HMODULE hModule)
	{
		//SpawnConsole(); // Spawn debug console; be aware that some anti-cheats are not happy when you create un-authorized window

		try
		{
			if (!component_loader::initialize())
			{
				return false;
			}
		}
		catch (std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
			return false;
		}

		if (!Initialize_Graphical_Overlay_Hook()) // Identify and then Hook graphical API if supported
		{
			goto EXIT_LIBRARY;
		}

		ShutdownListenerLoop(); // Loop waiting for shutdown's specific key strike
		component_loader::shutdown(); // shutdown phoenix components

		Shutdown_Graphical_Overlay_Hook(); // Revert back graphical API hooks

		Beep(220, 100); // Alarm user of phoenix being shutdown

EXIT_LIBRARY:
		FreeLibraryAndExitThread(hModule, 0); // And finally free the library and exit thread
	}
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);
	}

	return TRUE;
}