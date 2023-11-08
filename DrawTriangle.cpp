#include "DrawTriangle.h"

#pragma comment(lib, "d3dcompiler.lib")

void DrawTriangle::Initialize(HINSTANCE hInstance, int width, int height)
{
	D3DFramework::Initialize(hInstance, width, height);
	
	InitTriangle();
	InitPipeline();
}

void DrawTriangle::Destroy()
{
	mspInputLayout.Reset();
	mspVertexBuffer.Reset();
	mspPixelShader.Reset();
	mspVertexShader.Reset();

	D3DFramework::Destroy();
}

void DrawTriangle::InitTriangle()
{
	VERTEX vertices[]{
		{0.0f, 0.5f, 0.0f,{1.0f, 0.0f, 0.0f, 1.0f}},
		{0.5f, -0.5f, 0.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
		{-0.5f, -0.5f, 0.0f, {0.0f, 0.0f, 1.0f, 1.0f}}
	};
	CD3D11_BUFFER_DESC bd(sizeof(VERTEX) * 3,
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
		{"COLOR",0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	mspDevice->CreateInputLayout(
		ied, 2,
		spVS->GetBufferPointer(), spVS->GetBufferSize(),
		mspInputLayout.ReleaseAndGetAddressOf()
	);
	mspDeviceContext->IASetInputLayout(mspInputLayout.Get());
}

void DrawTriangle::Render()
{
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	mspDeviceContext->IASetVertexBuffers(0, 1, mspVertexBuffer.GetAddressOf(), &stride, &offset);
	mspDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mspDeviceContext->Draw(3, 0);
	
}
