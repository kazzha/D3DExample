#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>

#pragma comment (lib, "d3d11.lib")

const wchar_t gClassName[]{ L"D3DWindowClass" };
const wchar_t gTitle[]{ L"Direct3D Example" };
const int WINDOW_WIDTH{ 800 };
const int WINDOW_HEIGHT{ 600 };

HWND gHwnd{};
HINSTANCE gInstance{};

using namespace Microsoft::WRL;
// Interface DirectX Graphics
ComPtr<IDXGISwapChain> gspSwapChain{};
ComPtr<ID3D11Device> gspDevice{};
ComPtr<ID3D11DeviceContext> gspDeviceContext{};

ComPtr<ID3D11Texture2D> gspRenderTarget{};
ComPtr<ID3D11RenderTargetView> gspRenderTargetView{};

ComPtr<ID3D11Texture2D> gspDepthStencil{};
ComPtr<ID3D11DepthStencilView> gspDepthStencilView{};

void InitD3D();
void RenderFrame();
void DestroyD3D();


LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	WNDCLASSEX wc{};

	gInstance = hInstance;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = gClassName;
	wc.hInstance = gInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class!", L"Error", MB_OK);
		return 0;
	}

	RECT wr{ 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	gHwnd = CreateWindowEx(NULL,
		gClassName,
		gTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (gHwnd == NULL)
	{
		MessageBox(NULL, L"Failed to create window!", L"Error", MB_OK);
		return 0;
	}

	ShowWindow(gHwnd, nShowCmd);
	SetForegroundWindow(gHwnd); // 게임창을 제일 위에 올림
	SetFocus(gHwnd); // 마우스와 키보드(?)가 포커스된 윈도우로 감
	UpdateWindow(gHwnd);

	MSG msg{};
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
			
		}
		else
		{
			// GAME
			RenderFrame();
		}
	}

	DestroyD3D();

	return static_cast<int>(msg.wParam);
}

void InitD3D()
{
	DXGI_SWAP_CHAIN_DESC scd{};
	// ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC)); == 유니폼 초기화식
	scd.BufferCount = 1; // back buffer count만. front는 자동으로 침
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // UN: 부호없는 NORM: 노멀라이즈(정규화되어있다)
	scd.OutputWindow = gHwnd;
	scd.SampleDesc.Count = 1; // MSAA : Multi Sampling Anti-Aliasing (1개는 multi가 아님)
	scd.Windowed = TRUE; // 창모드

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&scd,
		gspSwapChain.ReleaseAndGetAddressOf(),
		gspDevice.ReleaseAndGetAddressOf(),
		nullptr,
		gspDeviceContext.ReleaseAndGetAddressOf()
	);

	gspSwapChain->GetBuffer(0, IID_PPV_ARGS(gspRenderTarget.ReleaseAndGetAddressOf()));
	gspDevice->CreateRenderTargetView(
		gspRenderTarget.Get(),
		nullptr,
		gspRenderTargetView.ReleaseAndGetAddressOf());


	// Depth Stencil Resource - Texture
	CD3D11_TEXTURE2D_DESC td(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL);
	
	gspDevice->CreateTexture2D(&td, nullptr, gspDepthStencil.ReleaseAndGetAddressOf()); // 만들기위해 주소로 준 것

	// Depth Stencil View
	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
	gspDevice->CreateDepthStencilView(gspDepthStencil.Get(), &dsvd,
		gspDepthStencilView.ReleaseAndGetAddressOf());

	// 파이프라인 설정
	gspDeviceContext->OMSetRenderTargets(1, gspRenderTargetView.GetAddressOf(),
		gspDepthStencilView.Get()); // *const *p어쩌고 = 포인터들의 배열로 넘기기 위해 주소로 준 것(gspRenderTargetView.GetAddressOf())

	CD3D11_VIEWPORT viewport(0.0f, 0.0f, 
		static_cast<FLOAT>(WINDOW_WIDTH), static_cast<FLOAT>(WINDOW_HEIGHT));

	gspDeviceContext->RSSetViewports(1, &viewport);
}
void RenderFrame()
{
}


void DestroyD3D()
{
	gspDepthStencilView.Reset();
	gspDepthStencil.Reset();
	gspRenderTargetView.Reset();
	gspRenderTarget.Reset();

	gspSwapChain.Reset();
	gspDevice.Reset();
	gspDeviceContext.Reset();

	DestroyWindow(gHwnd);
	UnregisterClass(gClassName, gInstance);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}