#pragma once
#include <d3d11.h>

_NODISCARD HRESULT LoadTextureFromFile(ID3D11Device* device,
    const wchar_t* filename, ID3D11ShaderResourceView** shaderResourceView,
    D3D11_TEXTURE2D_DESC* texture2dDesc);

//�_�~�[�e�N�X�`���̍쐬
_NODISCARD HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, DWORD value/*0xAABBGGRR*/, UINT dimension);


_NODISCARD HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** shaderResourceView);

void ReleaseAllTextures();