#pragma once
#include "D3DFramework.h"
#include <d3dcompiler.h>


class DrawTriangle : public D3DFramework
{
	struct VERTEX
	{
		FLOAT x, y, z;
		FLOAT color[4];
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout> mspInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mspVertexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> mspVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mspPixelShader;

public:
	void Initialize(HINSTANCE hInstance, int width = 800, int height = 600) override;
	void Destroy() override;

private:
	void InitTriangle();
	void InitPipeline();

protected:
	void Render() override;

};

