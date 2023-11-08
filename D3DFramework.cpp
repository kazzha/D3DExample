#include "D3DFramework.h"

#pragma comment (lib, "d3d11.lib")


void D3DFramework::InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wc{};

	mInstance = hInstance;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = CLASSNAME.c_str();
	wc.hInstance = mInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register window class!", L"Error", MB_OK);
		return;
	}

	RECT wr{ 0,0,mScreenWidth, mScreenHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	mHwnd = CreateWindowEx(NULL,
		CLASSNAME.c_str(),
		TITLE.c_str(),
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

	if (mHwnd == NULL)
	{
		MessageBox(NULL, L"Failed to create window!", L"Error", MB_OK);
		return;
	}

	SetWindowLongPtr(mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd); // 게임창을 제일 위에 올림
	SetFocus(mHwnd); // 마우스와 키보드(?)가 포커스된 윈도우로 감
	UpdateWindow(mHwnd);

}

void D3DFramework::InitD3D()
{
	DXGI_SWAP_CHAIN_DESC scd{};
	// ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC)); == 유니폼 초기화식
	scd.BufferCount = 1; // back buffer count만. front는 자동으로 침
	scd.BufferDesc.Width = mScreenWidth;
	scd.BufferDesc.Height = mScreenHeight;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // UN: 부호없는 NORM: 노멀라이즈(정규화되어있다)
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = mHwnd;
	scd.SampleDesc.Count = 1; // MSAA : Multi Sampling Anti-Aliasing (1개는 multi가 아님)
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.Windowed = TRUE; // 창모드

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&scd,
		mspSwapChain.ReleaseAndGetAddressOf(),
		mspDevice.ReleaseAndGetAddressOf(),
		nullptr,
		mspDeviceContext.ReleaseAndGetAddressOf()
	);

	OnResize();
}

void D3DFramework::OnResize()
{
	ID3D11RenderTargetView* nullViews[]{ nullptr };
	mspDeviceContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

	mspRenderTargetView.Reset();
	mspDepthStencilView.Reset();
	mspRenderTarget.Reset();
	mspDepthStencil.Reset();
	mspDeviceContext->Flush();

	mspSwapChain->ResizeBuffers(0, mScreenWidth, mScreenHeight, DXGI_FORMAT_UNKNOWN, 0);

	mspSwapChain->GetBuffer(0, IID_PPV_ARGS(mspRenderTarget.ReleaseAndGetAddressOf()));
	mspDevice->CreateRenderTargetView(
		mspRenderTarget.Get(),
		nullptr,
		mspRenderTargetView.ReleaseAndGetAddressOf());


	// Depth Stencil Resource - Texture
	CD3D11_TEXTURE2D_DESC td(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		mScreenWidth,
		mScreenHeight,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL);

	mspDevice->CreateTexture2D(&td, nullptr, mspDepthStencil.ReleaseAndGetAddressOf()); // 만들기위해 주소로 준 것

	// Depth Stencil View
	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
	mspDevice->CreateDepthStencilView(mspDepthStencil.Get(), &dsvd,
		mspDepthStencilView.ReleaseAndGetAddressOf());

	// 파이프라인 설정
	mspDeviceContext->OMSetRenderTargets(1, mspRenderTargetView.GetAddressOf(),
		mspDepthStencilView.Get()); // *const *p어쩌고 = 포인터들의 배열로 넘기기 위해 주소로 준 것(gspRenderTargetView.GetAddressOf())

	CD3D11_VIEWPORT viewport(0.0f, 0.0f,
		static_cast<FLOAT>(mScreenWidth), static_cast<FLOAT>(mScreenHeight));

	mspDeviceContext->RSSetViewports(1, &viewport);
}

void D3DFramework::RenderFrame()
{
	float bg[4]{ 0.0f, 0.2f, 0.4f, 1.0f };

	mspDeviceContext->ClearRenderTargetView(mspRenderTargetView.Get(), bg);
	mspDeviceContext->ClearDepthStencilView(mspDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 0);
	Render();
	mspSwapChain->Present(0, 0);
	
}

void D3DFramework::Initialize(HINSTANCE hInstance, int width, int height)
{
	mScreenWidth = width;
	mScreenHeight = height;

	InitWindow(hInstance);
	InitD3D();
}

void D3DFramework::Destroy()
{
	mspSwapChain->SetFullscreenState(FALSE, nullptr); // 창-전체 기능을 쓰려면 반드시 해제 전 창모드로
	// 돌아오기를 만들기

	mspDepthStencilView.Reset();
	mspDepthStencil.Reset();
	mspRenderTargetView.Reset();
	mspRenderTarget.Reset();

	mspSwapChain.Reset();
	mspDevice.Reset();
	mspDeviceContext.Reset();

	DestroyWindow(mHwnd);
	UnregisterClass(CLASSNAME.c_str(), mInstance);
}

void D3DFramework::GameLoop()
{
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
}

void D3DFramework::Render()
{
	
}

LRESULT D3DFramework::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:

		if (mResizing)
		{
			RenderFrame();
		}
		else
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);

		}
		break;

	case WM_ERASEBKGND:
		return 1;
		break;

	case WM_ENTERSIZEMOVE:
		mResizing = true;
		break;

	case WM_SIZE:
	{
		mScreenWidth = LOWORD(lParam);
		mScreenHeight = HIWORD(lParam);

		if (!mspDevice)
		{
			break;
		}

		if (wParam == SIZE_MINIMIZED)
		{
			mMinimized = true;
			mMaximized = false;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (mMinimized)
			{
				mMinimized = false;
				OnResize();
			}
			else if (mMaximized)
			{
				mMaximized = false;
				OnResize();
			}
			else if (mResizing)
			{

			}
			else
			{
				OnResize();
			}
		}

		break;
	}
	case WM_GETMINMAXINFO:
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 640;
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 480;
		break;

	case WM_EXITSIZEMOVE:
		mResizing = false;
		OnResize();
		break;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE); // 전체화면 벗어날때 소리 안나게
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pFramework = reinterpret_cast<D3DFramework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	return pFramework->MessageHandler(hwnd, message, wParam, lParam);
}
