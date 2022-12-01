#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")

class DirectX12 final
{
public:
	static bool Initialize();
	static void Shutdown();

private:
	static bool InstallHooks();

};
