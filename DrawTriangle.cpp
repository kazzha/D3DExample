#include "DrawTriangle.h"
#include <fstream>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

void DrawTriangle::Initialize(HINSTANCE hInstance, int width, int height)
{
	D3DFramework::Initialize(hInstance, width, height);
	
	InitTriangle();
	InitPipeline();

	CreateTextureFromBMP();
}

void DrawTriangle::Destroy()
{
	mspTextureView.Reset();
	mspTexture.Reset();
	mspInputLayout.Reset();
	mspBlendState.Reset();
	mspSamplerState.Reset();
	mspVertexBuffer.Reset();
	mspPixelShader.Reset();
	mspVertexShader.Reset();

	D3DFramework::Destroy();
}

void DrawTriangle::InitTriangle()
{
	VERTEX vertices[]{
		{-0.5f, 0.5f, 0.0f, 0.0f, 0.0f},
		{0.5f, 0.5f, 0.0f, 1.0f, 0.0f},
		{-0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
		{0.5f, -0.5f, 0.0f, 1.0f, 1.0f }
	};
	CD3D11_BUFFER_DESC bd(sizeof(vertices),
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE);

	mspDevice->CreateBuffer(&bd, nullptr, mspVertexBuffer.ReleaseAndGetAddressOf());

	D3D11_MAPPED_SUBRESOURCE ms;
	mspDeviceContext->Map(
		mspVertexBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&ms
	);

	memcpy(ms.pData, vertices, sizeof(vertices));
	mspDeviceContext->Unmap(mspVertexBuffer.Get(), 0);

	float border[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	CD3D11_SAMPLER_DESC sd(
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f,
		1,
		D3D11_COMPARISON_ALWAYS,
		border,
		0, 1
	);
	mspDevice->CreateSamplerState(&sd, mspSamplerState.ReleaseAndGetAddressOf());

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	mspDevice->CreateBlendState(&blendDesc, mspBlendState.ReleaseAndGetAddressOf());

	bd = CD3D11_BUFFER_DESC(
		sizeof(MatrixBuffer),
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE_DEFAULT
	);
	mspDevice->CreateBuffer(&bd, nullptr, mspConstantBuffer.ReleaseAndGetAddressOf());

	mX = mY = 0.0f;
	mRotationZ = 0.0f;
}

void DrawTriangle::InitPipeline()
{
	// Binary Large Object
	Microsoft::WRL::ComPtr<ID3DBlob> spVS;
	Microsoft::WRL::ComPtr<ID3DBlob> spPS;

	D3DCompileFromFile(
		L"VertexShader.hlsl",
		0, 0, "main", "vs_4_0_level_9_3",
		0, 0, spVS.GetAddressOf(),
		nullptr
	);

	D3DCompileFromFile(
		L"PixelShader.hlsl",
		0, 0, "main", "ps_4_0_level_9_3",
		0, 0, spPS.GetAddressOf(),
		nullptr
	);

	mspDevice->CreateVertexShader(spVS->GetBufferPointer(), spVS->GetBufferSize(),
		nullptr, mspVertexShader.ReleaseAndGetAddressOf());
	mspDevice->CreatePixelShader(spPS->GetBufferPointer(), spPS->GetBufferSize(),
		nullptr, mspPixelShader.ReleaseAndGetAddressOf());

	mspDeviceContext->VSSetShader(mspVertexShader.Get(), nullptr, 0);
	mspDeviceContext->PSSetShader(mspPixelShader.Get(), nullptr, 0);

	D3D11_INPUT_ELEMENT_DESC ied[]
	{
		{ "POSITION" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	mspDevice->CreateInputLayout(
		ied, 2,
		spVS->GetBufferPointer(), spVS->GetBufferSize(),
		mspInputLayout.ReleaseAndGetAddressOf()
	);
	mspDeviceContext->IASetInputLayout(mspInputLayout.Get());
	
	mspDeviceContext->VSSetConstantBuffers(0, 1, mspConstantBuffer.GetAddressOf());
}

HRESULT DrawTriangle::CreateTextureFromBMP()
{
	std::ifstream ifs;
	ifs.open("Textures/32.bmp", std::ifstream::binary);

	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	ifs.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));
	ifs.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));

	std::vector<char> pixels(bih.biSizeImage);
	ifs.seekg(bfh.bfOffBits);
	int pitch = bih.biWidth * (bih.biBitCount / 8);

	char r{}, g{}, b{}, a{};
	int index{};
	/*
	for (int y = bih.biHeight - 1; y >= 0; y--)
	{
		ifs.read(&pixels[y * pitch], pitch);
	}
	*/
	for (int y = bih.biHeight - 1; y >= 0; y--)
	{
		index = y * pitch;
		for (int x = 0; x < bih.biWidth; x++)
		{
			ifs.read(&b, 1);
			ifs.read(&g, 1);
			ifs.read(&r, 1);
			ifs.read(&a, 1);

			if (static_cast<unsigned char>(r) == 30 && 
				static_cast<unsigned char>(g) == 199 &&
				static_cast<unsigned char>(b) == 250)
			{
				pixels[index] = pixels[index + 1] = pixels[index + 2] = pixels[index + 3] = 0;
			}
			else
			{
				pixels[index] = b;
				pixels[index + 1] = g;
				pixels[index + 2] = r;
				pixels[index + 3] = a;
			}
			index += 4;
		}
	}

	ifs.close();

	CD3D11_TEXTURE2D_DESC td(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		bih.biWidth, bih.biHeight,
		1, 1
	);

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &pixels[0];
	initData.SysMemPitch = pitch;
	initData.SysMemSlicePitch = 0;
	mspDevice->CreateTexture2D(&td, &initData, mspTexture.ReleaseAndGetAddressOf());

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(
		D3D11_SRV_DIMENSION_TEXTURE2D,
		td.Format,
		0, 1
	);
	mspDevice->CreateShaderResourceView(mspTexture.Get(), &srvd, mspTextureView.ReleaseAndGetAddressOf());


	return S_OK;
}

void DrawTriangle::Update(float delta)
{
	if (mInput.IsKeyDown('Q'))
	{
		mRotationZ += DirectX::XM_PI * delta; // radian, time-base == 1초에 180도 + 시계방향
	}
	else if (mInput.IsKeyDown('E'))
	{
		mRotationZ -= DirectX::XM_PI * delta; // - 반시계방향
	}

	if (mInput.IsKeyDown(VK_UP) || mInput.IsKeyDown('W'))
	{
		mY += 1.0f * delta;
	}
	else if (mInput.IsKeyDown(VK_DOWN))
	{
		mY -= 1.0f * delta;
	}
	if (mInput.IsKeyDown(VK_LEFT)) 
	{
		mX -= 1.0f * delta;
	}
	else if (mInput.IsKeyDown(VK_RIGHT))
	{
		mX += 1.0f * delta;
	}

	if (mInput.IsKeyDown('1'))
	{
		mTimer.SetScale(1.0f);
	}
	else if (mInput.IsKeyDown('2'))
	{
		mTimer.SetScale(2.0f);
	}
	else if (mInput.IsKeyDown('3'))
	{
		mTimer.SetScale(0.5f);
	}

	mWorld = DirectX::XMMatrixIdentity();
	mWorld *= DirectX::XMMatrixRotationZ(mRotationZ);
	mWorld *= DirectX::XMMatrixTranslation(mX, mY, 0.0f);
}

void DrawTriangle::Render()
{
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;


	mspDeviceContext->IASetVertexBuffers(0, 1, mspVertexBuffer.GetAddressOf(), &stride, &offset);
	mspDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mspDeviceContext->PSSetSamplers(0, 1, mspSamplerState.GetAddressOf());
	mspDeviceContext->PSSetShaderResources(0, 1, mspTextureView.GetAddressOf());
	mspDeviceContext->OMSetBlendState(mspBlendState.Get(), nullptr, 0xffffffff);
	
	MatrixBuffer mb;
	mb.world = DirectX::XMMatrixTranspose(mWorld);
	mspDeviceContext->UpdateSubresource(mspConstantBuffer.Get(), 0, nullptr, &mb, 0, 0);
	
	mspDeviceContext->Draw(4, 0);

	
}
