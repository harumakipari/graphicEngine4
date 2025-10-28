#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Graphics/Core/ConstantBuffer.h"

class LightManager
{
public:
    struct PointLight
    {
        DirectX::XMFLOAT4 position{ 0.0f,0.0f,0.0f,0.0f };
        DirectX::XMFLOAT4 color{ 1.0f,0.0f,0.0f,1.0f };
        float range = 0.5f;
        float pads[3] = {};
    };

    struct LightConstants
    {
        DirectX::XMFLOAT4 lightDirection = {};
        DirectX::XMFLOAT4 lightColor = {};
        float iblIntensity = 1.0f;
        int directionalLightEnable = 1;// ���s������ on / off
        int pointLightEnable = 1;
        int pointLightCount = 0;
        PointLight pointsLight[8];
    };

public:
    void Initialize(ID3D11Device* device);
    void Update(float deltaTime);
    void Apply(ID3D11DeviceContext* context, int slot) const;

    void AddPointLight(const PointLight& light)
    {
        pointLights.push_back(light);
    }

    void SetDirectionalLight(const DirectX::XMFLOAT4& dir, const DirectX::XMFLOAT4& color)
    {
        lightDirection = dir;
        lightColor = color;
    }

    void DrawGUI();

private:
    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,4.1f };
    float iblIntensity = 2.0f;  //Image Basesd Lighting�̋��x

    DirectX::XMFLOAT4 pointLightPosition[8] =
    {
        { -2.0f,  2.0f, 0.0f, 10.0f },
        { -1.0f,  2.0f, 0.0f, 10.0f },
        { 0.0f,  2.0f, 0.0f, 10.0f },
        { 1.0f,  2.0f, 0.0f, 10.0f },
        { 2.0f,  2.0f, 0.0f, 10.0f },
        { 3.0f,  2.0f, 0.0f, 10.0f },
        { 4.0f,  2.0f, 0.0f, 10.0f },
        { 5.0f,  2.0f, 0.0f, 10.0f },
    };

    DirectX::XMFLOAT4 pointLightColor[8] =
    {
        { 1.0f, 0.0f, 0.0f, 10.0f },  // ��
        { 0.0f, 1.0f, 0.0f, 10.0f },  // ��
        { 0.0f, 0.0f, 1.0f, 10.0f },  // ��
        { 1.0f, 1.0f, 0.0f, 10.0f },  // ��
        { 1.0f, 0.0f, 1.0f, 10.0f },  // �}�[���^
        { 0.0f, 1.0f, 1.0f, 10.0f },  // �V�A��
        { 1.0f, 0.5f, 0.0f, 10.0f },  // �I�����W
        { 0.5f, 0.0f, 1.0f, 10.0f },  // ��
    };

    float pointLightRange[8] =
    {
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
    };
    bool directionalLightEnable = true; // ���s������ on / off
    bool pointLightEnable = true;
    int pointLightCount = 8;

    LightConstants constants = {};
    std::vector<PointLight> pointLights;
    std::unique_ptr<ConstantBuffer<LightConstants>> lightCBuffer;
};