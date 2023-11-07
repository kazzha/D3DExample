#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <string>


class D3DFramework
{
	const std::wstring gClassName{ L"D3DWindowClass" };
	const std::wstring gTitle{ L"Direct3D Example" };

protected:
	int gScreenWidth{ 800 };
	int gScreenHeight{ 600 };
	bool gMinimized{ false };
	bool gMaximized{ false };
	bool gResizing{ false };

	HWND gHwnd{};
	HINSTANCE gInstance{};

	// Interface DirectX Graphics
	Microsoft::WRL::ComPtr<IDXGISwapChain> gspSwapChain{};
	Microsoft::WRL::ComPtr<ID3D11Device> gspDevice{};
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> gspDeviceContext{};

	Microsoft::WRL::ComPtr<ID3D11Texture2D> gspRenderTarget{};
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gspRenderTargetView{};

	Microsoft::WRL::ComPtr<ID3D11Texture2D> gspDepthStencil{};
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> gspDepthStencilView{};

private:
	void InitWindow(HINSTANCE hInstance);
	void InitD3D();

protected:
	void OnResize();
	void RenderFrame();

public:
	virtual void Initialize(HINSTANCE hInstance, int width = 800, int height = 600);
	virtual void DestroyD3D();
	void GameLoop();

public:
	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
