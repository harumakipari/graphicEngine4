#pragma once

// デカール用データ
struct decal_data
{
    DirectX::XMFLOAT3	translation{ 0, 0, 0 };
    DirectX::XMFLOAT3	scaling{ 1, 1, 1 };
    DirectX::XMFLOAT3	rotation{ 0, 0, 0 };
    DirectX::XMFLOAT4	color{ 1, 1, 1, 1 };
    int decal_index = -1;
};
