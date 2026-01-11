#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Stage/ClothSimulate.h"



class MainScene : public SceneBase
{
public:
    struct OdenSoupConstantBuffer
    {
        float normalScale = 7.36f;
        float normalStrength = 2.11f;
        float normalSpeed = 0.56f;
        float specularSmoothness = 0.229f;

        DirectX::XMFLOAT3 mainLightColor = { 0.3f, 0.3f, 0.3f };
        float specularHardness = 0.215f; // 0..1 slider to blend soft->hard

        DirectX::XMFLOAT3 specularColor = { 0.3f, 0.3f, 0.3f };
        float waterAlpha = 0.8f;

        DirectX::XMFLOAT3 horizonColor = { 0.75f, 0.90f, 1.0f };
        float specularIntensity = 5.74f; // 全体スケール

        DirectX::XMFLOAT4 shallowColor = { 0.70f, 0.90f, 1.00f, 1.0f }; // 明るい浅瀬

        DirectX::XMFLOAT4 deepColor = {0.05f, 0.28f, 0.65f, 1.0f};
    };
    

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<MainScene> _autoenrollment;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> waterNormalTexture; // ノーマルテクスチャ

    // おでんの汁の定数バッファ
    std::unique_ptr<ConstantBuffer<OdenSoupConstantBuffer>> odenSoupCBuffer;
    OdenSoupConstantBuffer odenSoupConstantBuffer;
private:
    std::unique_ptr<ClothSimulate> clothSimulate;

};
