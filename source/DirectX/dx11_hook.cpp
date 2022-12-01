/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Directx11 Hook
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-2
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <dx11_hook.hpp>
#include <input_hook.hpp>


HWND DirectX11::window = NULL;
ID3D11Device* DirectX11::p_device;
ID3D11DeviceContext* DirectX11::p_context;
ID3D11RenderTargetView* DirectX11::mainRenderTargetView;

LPVOID* DirectX11::p_vtable;

DirectX11::Present PresentOriginal = nullptr;
DirectX11::ResizeBuffers ResizeBuffersOriginal = nullptr;

HRESULT WINAPI DirectX11::PresentHook(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
{
	static std::once_flag p_isInitialized;

	std::call_once(p_isInitialized, [&]() {
		log_debug_string("Present Hook: Initializing first time Run Requirements");

		if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device)))
		{
			p_device->GetImmediateContext(&p_context);

			DXGI_SWAP_CHAIN_DESC sd;
			p_swap_chain->GetDesc(&sd);
			window = sd.OutputWindow;

#pragma warning(push)
#pragma warning(disable : 6387)
			ID3D11Texture2D* pBackBuffer;
			p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
#pragma warning(pop)
		}
	});

	component_loader::render_frame();

	return PresentOriginal(p_swap_chain, sync_interval, flags);
}

HRESULT WINAPI DirectX11::ResizeBuffersHook(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags)
{
	if (mainRenderTargetView) {
		p_context->OMSetRenderTargets(0, 0, 0);
		mainRenderTargetView->Release();
	}

	HRESULT hr = ResizeBuffersOriginal(p_swap_chain, buffer_count, width, height, new_format, swap_chain_flags);

#pragma warning(push)
#pragma warning(disable : 6387)
	ID3D11Texture2D* pBackBuffer;
	hr = p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	hr = p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
	pBackBuffer->Release();
#pragma warning(pop)

	p_context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	//vp.Width = Width;
	//vp.Height = Height;
	vp.Width = *reinterpret_cast<float*>(&width);
	vp.Height = *reinterpret_cast<float*>(&height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	p_context->RSSetViewports(1, &vp);
	return hr;
}

bool DirectX11::RetrieveInfo()
{
	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

	WNDCLASSEXA windowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "PHOENIX", NULL };
	RegisterClassExA(&windowClass);
	HWND tempWindow = CreateWindow("PHOENIX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, windowClass.hInstance, NULL);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = tempWindow;
	swapChainDesc.Windowed = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	IDXGISwapChain* swapChain;
	ID3D11Device* device;

	if (D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		featureLevels,
		2,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		nullptr,
		nullptr) != S_OK)
	{
		// Error on Retrieving Information by VirtualTable Method

		DestroyWindow(tempWindow);
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

		return false;
	}

	p_vtable = *reinterpret_cast<LPVOID**>(swapChain);

	log_debug_string("DirectX11 swapChain vtable[ 8] ---> Present Address: 0x%llX", p_vtable[8]);
	log_debug_string("DirectX11 swapChain vtable[13] ---> ResizeBuffers Address: 0x%llX", p_vtable[13]);

	swapChain->Release();
	device->Release();

	return true;
}

void DirectX11::FailSafeMechanism()
{
//	uintptr_t fnIDXGISwapChainPresent = 0x000000000;
//	DWORD_PTR hDxgi = (DWORD_PTR)GetModuleHandle("dxgi.dll");
//#if defined(_WIN64)
//	fnIDXGISwapChainPresent = (DWORD_PTR)hDxgi + 0x5070;
//#elif defined (_WIN32)
//	fnIDXGISwapChainPresent = (DWORD_PTR)hDxgi + 0x10230;
//#endif
//
//	log_debug_string("** DirectX11 Fail-Safe Mechanism ** Present Address: 0x%llX", fnIDXGISwapChainPresent);

	HWND hWnd = utils::common::getCurrentProcessWindowHandle();

	log_debug_string("** DirectX11 Fail-Safe Mechanism ** Foreground Window: 0x%llX(%s)", hWnd, utils::common::GetWindowTitleString(hWnd).c_str());
}

bool DirectX11::InstallHooks()
{
	if (MH_Initialize() != MH_OK) { return false; }


	log_debug_string("Detouring DirectX11 Present from original 0x%llX to 0x%llX", p_vtable[8], &DirectX11::PresentHook);
	if (MH_CreateHook((DWORD_PTR*)p_vtable[8], &DirectX11::PresentHook, reinterpret_cast<void**>(&PresentOriginal)) != MH_OK) { return false; }
	if (MH_EnableHook((DWORD_PTR*)p_vtable[8]) != MH_OK) { return false; }

	log_debug_string("Detouring DirectX11 ResizeBuffersHook from original 0x%llX to 0x%llX", p_vtable[13], &DirectX11::ResizeBuffersHook);
	if (MH_CreateHook((DWORD_PTR*)p_vtable[13], &DirectX11::ResizeBuffersHook, reinterpret_cast<void**>(&ResizeBuffersOriginal)) != MH_OK) { return false; }
	if (MH_EnableHook((DWORD_PTR*)p_vtable[13]) != MH_OK) { return false; }

	return true;
}

bool DirectX11::WindowRelated()
{
	do { // or just call these directly at end of present hook initiation 
		Sleep(100);
	} while (!window);

	if (InputHook::Initialize(window) == false) {
		log_error_string("Problem on Hooking DirectX11 Window Input Listener");
		return false;
	}

	return true;
}

bool DirectX11::Initialize()
{
	if (RetrieveInfo() == false) {
		log_error_string("Retrieving DirectX11 Device Information Failed");
		return false;
	}
	if (InstallHooks() == false) {
		log_error_string("Hooking DirectX11 API Functions Failed");
		return false;
	}
	if (WindowRelated() == false) {
		return false;
	}

	FailSafeMechanism();

	return true;
}

void DirectX11::Shutdown()
{
	InputHook::Shutdown(window);

	p_device->Release();
	p_context->Release();

	//MH_DisableHook(MH_ALL_HOOKS);

	//MH_RemoveHook((DWORD_PTR*)p_vtable[8]);
	//MH_RemoveHook((DWORD_PTR*)p_vtable[13]);

	//MH_Uninitialize();

	if (MH_DisableHook(MH_ALL_HOOKS)) { return; };
	if (MH_Uninitialize()) { return; }
}