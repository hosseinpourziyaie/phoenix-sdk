#pragma once

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

class DirectX11 final
{
public:
	static bool Initialize();
	static void Shutdown();

	typedef HRESULT(__stdcall* Present) (IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
	typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);

	static HRESULT WINAPI PresentHook(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
	static HRESULT WINAPI ResizeBuffersHook(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);

	static HWND window;
	static ID3D11Device* p_device;
	static ID3D11DeviceContext* p_context;
	static ID3D11RenderTargetView* mainRenderTargetView;

private:
	static LPVOID* p_vtable;

	static bool RetrieveInfo();
	static void FailSafeMechanism();
	static bool InstallHooks();
	static bool WindowRelated();
};
