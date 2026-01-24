#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Stage/ClothSimulate.h"
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenData/OdenGameParameter.h"


class OdenSlotManager;
class OdenSlotActor;

class TutorialScene : public SceneBase
{
public:
    struct OdenSoupConstantBuffer
    {
        float normalScale = 7.36f;
        float normalStrength = 2.11f;
        float normalSpeed = 0.56f;
        float specularSmoothness = 0.174f;

        DirectX::XMFLOAT3 mainLightColor = { 0.8f, 0.8f, 0.8f };
        float specularHardness = 0.062f; // 0..1 slider to blend soft->hard

        DirectX::XMFLOAT3 specularColor = { 0.8f, 0.8f, 0.8f };
        float waterAlpha = 0.8f;

        DirectX::XMFLOAT4 shallowColor = { 0.898f, 0.764f, 0.288f, 1.0f };

        DirectX::XMFLOAT4 deepColor = { 1.0f, 0.72f, 0.1f, 1.0f };

        float specularIntensity = 3.1f; // 全体スケール
        float turbidity = 0.5f; // 濁り
        float oilStrength = 0.5f; // 油膜

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
    static inline Scene::Autoenrollment<TutorialScene> _autoenrollment;

private:
    const OrderEntry* FindOrderByUIName(const std::string& uiName)
    {
        for (const auto& o : OdenGameParameter::orderDB.shapeOrders)
        {
            if (o.uiName == uiName)
                return &o;
        }
        for (const auto& o : OdenGameParameter::orderDB.ingredientOrders)
        {
            if (o.uiName == uiName)
                return &o;
        }
        return nullptr;
    }

    void CreateSlotRow(ActorManager* actorManager,
        const std::string& rowName,
        const std::vector<std::string>& ingredients,
        float startZ,
        ERotationType rotationType);

private:

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> waterNormalTexture; // ノーマルテクスチャ

    // おでんの汁の定数バッファ
    std::unique_ptr<ConstantBuffer<OdenSoupConstantBuffer>> odenSoupCBuffer;
    OdenSoupConstantBuffer odenSoupConstantBuffer;
private:
    std::unique_ptr<ClothSimulate> clothSimulate;

    GameDifficulty difficulty = GameDifficulty::Hard;

    std::shared_ptr<OdenSlotManager> slotManager;
    
};
