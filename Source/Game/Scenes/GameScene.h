#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"


#include "Graphics/Renderer/SceneRenderer.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Player/Player.h"
#include "Game/ScissorsGame/NeedleEnemyActor.h"
#include "Game/ScissorsGame/YarnEnemyActor.h"


class GameScene : public SceneBase
{
public:
    //　敵の調整
    struct EnemyTuning
    {
        // 吹っ飛び系
        float knockbackDistanceDash = 8.0f;
        float knockbackDistanceNormal = 5.0f;

        float knockbackHeightDash = 5.0f;
        float knockbackHeightNormal = 8.0f;

        float knockbackDurationDash = 0.7f;
        float knockbackDurationNormal = 0.9f;

        // 発光系
        float flashDuration = 0.7f;
        float emissivePower = 20.0f;
        float flashSharpness = 7.0f;
    };

    // コインの調整　
    struct CoinTuning
    {
        float duration = 0.9f; // 演出に掛ける時間
        float height = 2.3f;

        // ===== 上昇トレイル =====
        float trailSpawnInterval = 0.15f;
        float trailSize = 15.0f;

        // ===== バースト =====
        int burstCount = 10;
        float burstSize = 40.0f;
        float burstShrinkSpeed = 500.0f;
    };

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    // 定数バッファの更新処理をシーンごとにカスタマイズできるようにするための仮想関数
    void UpdateConstants(ID3D11DeviceContext* immediateContext, float deltaTime)override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<GameScene> _autoenrollment;

private:
    void SpawnEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 } );

    void SpawnBigEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 } );

public:
    EnemyTuning enemyTuning = {}; // 敵の調整用
    CoinTuning coinTuning = {}; // 敵の調整用

private:

    TPSCameraComponent* mainCameraComponent = nullptr;
    std::shared_ptr<ScissorsPlayer1> player;
    std::shared_ptr<NeedleEnemyActor> needleEnemyActor;



};
