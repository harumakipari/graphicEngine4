#pragma once
#include <wrl.h>
#include <d3d11.h>

[[nodiscard]] HRESULT CreateVsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11VertexShader** vertexShader,
    ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc,
    UINT numElements);

[[nodiscard]] HRESULT CreatePsFromCSO(ID3D11Device* device,
    const char* csoName, ID3D11PixelShader** pixelShader);

[[nodiscard]] HRESULT CreateGsFromCSO(ID3D11Device* device, const char* csoName, ID3D11GeometryShader** geometryShader);

[[nodiscard]] HRESULT CreateCsFromCSO(ID3D11Device* device, const char* csoName, ID3D11ComputeShader** computeShader);
