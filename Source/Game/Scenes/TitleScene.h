#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Stage/ClothSimulate.h"


class OdenCameraTargetActor;

class TitleScene : public SceneBase
{
public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<TitleScene> _autoenrollment;

private:
    // カメラのターゲットを移動する
    void MoveCameraTarget(const XMFLOAT3 originPos, const XMFLOAT3 targetPos);
private:
    // カメラのターゲットアクター
    std::shared_ptr<OdenCameraTargetActor> mainCameraTarget;

    std::unique_ptr<ClothSimulate> clothSimulate[5];
    
    XMFLOAT3 titleCameraTargetPos = { -12.3f,13.8f,-12.5f };
    XMFLOAT3 selectCameraTargetPos = { -3.6f,5.7f,0.3f };
};
