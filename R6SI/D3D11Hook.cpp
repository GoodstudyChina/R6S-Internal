#include <Windows.h>
#include <d3d11.h>
#include "Helpers.h"

#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <fstream>

#include "FW1FontWrapper/FW1FontWrapper.h"
#include "BeaEngine/BeaEngine.h"
#include "BeaEngine/Includes/basic_types.h"
#include "BeaEngine/Includes/export.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "BeaEngine64.lib")

#include "Renderer.h"

IDXGISwapChain* pSwapChain = NULL;
ID3D11RenderTargetView* pRenderTargetView = NULL;

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall *D3D11ClearRenderTargetViewHook) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4]);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pDeviceContextVTable = NULL;

D3D11PresentHook phookD3D11Present = NULL;
D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
D3D11ClearRenderTargetViewHook phookD3D11ClearRenderTargetView = NULL;

IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

bool firstTime = true;
void* detourBuffer[3];

using namespace std;

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
		pDevice->GetImmediateContext(&pContext);

		FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);

		//create font
		HRESULT hr;
		hr = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		if (FAILED(hr)) { 
			Helpers::LogError("FW1CreateFactory");
			Helpers::Log("");
			return hr; 
		}
		else {
			Helpers::LogSuccess("FW1CreateFactory");
			Helpers::Log("");
		}

		hr = pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);
		if (FAILED(hr)) { 
			Helpers::LogError("CreateFontWrapper");
			return hr; 
		}
		else {
			Helpers::LogSuccess("CreateFontWrapper");
			Helpers::Log("");
		}

		pFW1Factory->Release();
		hr = pFW1Factory->Release();
		if (FAILED(hr)) {
			Helpers::LogError("pFW1Factory Released");
			Helpers::Log("");
			return hr;
		}
		else {
			Helpers::LogSuccess("pFW1Factory Released");
			Helpers::Log("");
		}

		Helpers::Log("hookD3D11Present");
		Helpers::Log("");

		firstTime = false;
	}

	pFontWrapper->DrawString(pContext, L"D3D11 Hook", 10.0f, 16.0f, 16.0f, 0xffff1612, FW1_RESTORESTATE);

	/*Renderer::GetInstance()->DrawTest();*/

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hookD3D11ClearRenderTargetView(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4])
{
	return phookD3D11ClearRenderTargetView(pContext, pRenderTargetView, ColorRGBA);
}

const void* DetourFuncVTable(SIZE_T* src, const BYTE* dest, const DWORD index)
{
	DWORD dwVirtualProtectBackup;
	SIZE_T* const indexPtr = &src[index];
	const void* origFunc = (void*)*indexPtr;
	VirtualProtect(indexPtr, sizeof(SIZE_T), PAGE_EXECUTE_READWRITE, &dwVirtualProtectBackup);
	*indexPtr = (SIZE_T)dest;
	VirtualProtect(indexPtr, sizeof(SIZE_T), dwVirtualProtectBackup, &dwVirtualProtectBackup);
	return origFunc;
}

#ifdef _WIN64
struct HookContext
{
	BYTE original_code[64];
	SIZE_T dst_ptr;
	BYTE far_jmp[6];
};

HookContext* presenthook64;

const unsigned int DisasmLengthCheck(const SIZE_T address, const unsigned int jumplength)
{
	DISASM disasm;
	memset(&disasm, 0, sizeof(DISASM));

	disasm.EIP = (UIntPtr)address;
	disasm.Archi = 0x40;

	unsigned int processed = 0;
	while (processed < jumplength)
	{
		const int len = Disasm(&disasm);
		if (len == UNKNOWN_OPCODE)
		{
			++disasm.EIP;
		}
		else
		{
			processed += len;
			disasm.EIP += len;
		}
	}

	return processed;
}

const void* DetourFunc64(BYTE* const src, const BYTE* dest, const unsigned int jumplength)
{
	// Allocate a memory page that is going to contain executable code.
	MEMORY_BASIC_INFORMATION mbi;
	for (SIZE_T addr = (SIZE_T)src; addr > (SIZE_T)src - 0x80000000; addr = (SIZE_T)mbi.BaseAddress - 1)
	{
		if (!VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)))
		{
			break;
		}

		if (mbi.State == MEM_FREE)
		{
			if (presenthook64 = (HookContext*)VirtualAlloc(mbi.BaseAddress, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
			{
				break;
			}
		}
	}

	// If allocating a memory page failed, the function failed.
	if (!presenthook64)
	{
		return NULL;
	}

	// Select a pointer slot for the memory page to be freed on unload.
	for (int i = 0; i < sizeof(detourBuffer) / sizeof(void*); ++i)
	{
		if (!detourBuffer[i])
		{
			detourBuffer[i] = presenthook64;
			break;
		}
	}

	BYTE detour[] = { 0x50, 0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x87, 0x04, 0x24, 0xC3 };
	const unsigned int length = DisasmLengthCheck((SIZE_T)src, jumplength);
	memcpy(presenthook64->original_code, src, length);
	memcpy(&presenthook64->original_code[length], detour, sizeof(detour));
	*(SIZE_T*)&presenthook64->original_code[length + 3] = (SIZE_T)src + length;

	// Build a far jump to the destination function.
	*(WORD*)&presenthook64->far_jmp = 0x25FF;
	*(DWORD*)(presenthook64->far_jmp + 2) = (DWORD)((SIZE_T)presenthook64 - (SIZE_T)src + FIELD_OFFSET(HookContext, dst_ptr) - 6);
	presenthook64->dst_ptr = (SIZE_T)dest;

	// Write the hook to the original function.
	DWORD flOld = 0;
	VirtualProtect(src, 6, PAGE_EXECUTE_READWRITE, &flOld);
	memcpy(src, presenthook64->far_jmp, sizeof(presenthook64->far_jmp));
	VirtualProtect(src, 6, flOld, &flOld);

	// Return a pointer to the original code.
	return presenthook64->original_code;
}
#else
const void* DetourFunc(BYTE* const src, const BYTE* dest, const DWORD length)
{
	BYTE* jump = new BYTE[length + 5];
	for (int i = 0; i < sizeof(detourBuffer) / sizeof(void*); ++i)
	{
		if (!detourBuffer[i])
		{
			detourBuffer[i] = jump;
			break;
		}
	}

	DWORD dwVirtualProtectBackup;
	VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

	memcpy(jump, src, length);
	jump += length;

	jump[0] = 0xE9;
	*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;

	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

	VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

	return jump - length;
}
#endif

DWORD __stdcall InitializeHook(LPVOID)
{
	AllocConsole();
	freopen("CON", "w", stdout);
	SetConsoleTitleA("R6SI - Debug");

	Helpers::LogSuccess("Cheat Initialised");
	Helpers::Log("");

	HWND hWnd = GetForegroundWindow();
	IDXGISwapChain* pSwapChain;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
	{
		Helpers::LogError("directX device and swapchain!");
		Helpers::Log("");
		return NULL;
	}

	Helpers::LogSuccess("directX device and swapchain!");
	Helpers::Log("");

	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	pDeviceContextVTable = (DWORD_PTR*)pContext;
	pDeviceContextVTable = (DWORD_PTR*)pDeviceContextVTable[0];


#ifdef _WIN64
	phookD3D11Present = (D3D11PresentHook)DetourFunc64((BYTE*)pSwapChainVtable[8], (BYTE*)hookD3D11Present, 16);
	phookD3D11DrawIndexed = (D3D11DrawIndexedHook)DetourFunc64((BYTE*)pDeviceContextVTable[12], (BYTE*)hookD3D11DrawIndexed, 16);
	phookD3D11ClearRenderTargetView = (D3D11ClearRenderTargetViewHook)DetourFunc64((BYTE*)pDeviceContextVTable[50], (BYTE*)hookD3D11ClearRenderTargetView, 16);

	Helpers::Log("WIN64");
	Helpers::Log("");
#else
	phookD3D11Present = (D3D11PresentHook)DetourFunc((BYTE*)pSwapChainVtable[8], (BYTE*)hookD3D11Present, 5);
	phookD3D11DrawIndexed = (D3D11DrawIndexedHook)DetourFunc((BYTE*)pDeviceContextVTable[12], (BYTE*)hookD3D11DrawIndexed, 5);
	phookD3D11ClearRenderTargetView = (D3D11ClearRenderTargetViewHook)DetourFunc((BYTE*)pDeviceContextVTable[50], (BYTE*)hookD3D11ClearRenderTargetView, 5);

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	Helpers::Log("WIN32");
	Helpers::Log("");
#endif

	DWORD_PTR*  pSwapChainVT = reinterpret_cast<DWORD_PTR*>(pSwapChain);
	DWORD_PTR*  pDeviceVT = reinterpret_cast<DWORD_PTR*>(pDevice);
	DWORD_PTR*  pContextVT = reinterpret_cast<DWORD_PTR*>(pContext);

	pSwapChainVT = reinterpret_cast<DWORD_PTR*>(pSwapChainVT[0]);
	pDeviceVT = reinterpret_cast<DWORD_PTR*>(pDeviceVT[0]);
	pContextVT = reinterpret_cast<DWORD_PTR*>(pContextVT[0]);

	Helpers::LogAddress("pSwapChainVT", reinterpret_cast<int64_t>(pSwapChainVT));
	Helpers::LogAddress("pDeviceVT", reinterpret_cast<int64_t>(pDeviceVT));
	Helpers::LogAddress("pContextVT", reinterpret_cast<int64_t>(pContextVT));
	Helpers::Log("");
	Helpers::LogAddress("D3D11SWAPCHAIN", pSwapChainVT[8]);
	Helpers::LogAddress("D3D11CONTEXT", pContextVT[12]);
	Helpers::LogAddress("D3D11DEVICE", pDeviceVT[24]);
	Helpers::Log("");

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	Helpers::LogSuccess("pDevice, pContext and pSwapChain Released");
	Helpers::Log("");

	return NULL;
}


BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		memset(detourBuffer, 0, sizeof(detourBuffer) * sizeof(void*));
		CreateThread(NULL, 0, InitializeHook, NULL, 0, NULL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (pFontWrapper)
		{
			pFontWrapper->Release();
	}

		for (int i = 0; i < sizeof(detourBuffer) / sizeof(void*); ++i)
		{
			if (detourBuffer[i])
			{
#ifdef _WIN64
				VirtualFree(detourBuffer[i], 0, MEM_RELEASE);
#else
				delete[] detourBuffer[i];
#endif
			}
		}
}

	return TRUE;
}