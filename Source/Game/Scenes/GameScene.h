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
#include "Game/ScissorsGame/YarnEnemyActor.h"


class GameScene : public SceneBase
{
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

private:

    TPSCameraComponent* mainCameraComponent = nullptr;
    std::shared_ptr<ScissorsPlayer1> player;



};
