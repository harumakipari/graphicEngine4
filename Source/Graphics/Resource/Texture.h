#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

#include "Engine/Utility/Win32Utils.h"

_NODISCARD HRESULT LoadTextureFromFile(ID3D11Device* device,
                                       const wchar_t* filename, ID3D11ShaderResourceView** shaderResourceView,
                                       D3D11_TEXTURE2D_DESC* texture2dDesc);

//ダミーテクスチャの作成
_NODISCARD HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value/*0xAABBGGRR*/, UINT dimension);

_NODISCARD HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView);

template<typename T>
HRESULT create_structured_buffer_shader_resource_view(ID3D11Device* device, const std::vector<T>& data, ID3D11ShaderResourceView** shaderResourceView)
{
	// Describe the structured buffer
	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(T) * data.size());
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buffer_desc.StructureByteStride = sizeof(T);

	// Provide initial data for buffer creation
	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = data.data();

	// Create the structured buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = device->CreateBuffer(&buffer_desc, &subresource_data, buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// Describe the SRV for this buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN; // Must be unknown for structured buffers
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.FirstElement = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(data.size());

	// Create the SRV
	hr = device->CreateShaderResourceView(buffer.Get(), &shader_resource_view_desc, shaderResourceView);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}


void ReleaseAllTextures();