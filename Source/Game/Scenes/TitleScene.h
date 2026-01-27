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
    enum class TitlePhase :uint8_t
    {
        StartWait,        // スタート前（看板は無効）
        CameraMovingIn,   // カメラ寄り中
        DifficultySelect, // 難易度選択中（有効）
        CameraMovingOut   // 戻り中
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
    static inline Scene::Autoenrollment<TitleScene> _autoenrollment;

public:
    TitlePhase GetPhase() const { return phase; }

    void SetPhase(const TitlePhase phase) { this->phase = phase; }


private:
    // カメラのターゲットアクター
    std::shared_ptr<OdenCameraTargetActor> mainCameraTarget;
    std::shared_ptr<MainCamera> mainCameraActor;

    std::unique_ptr<ClothSimulate> clothSimulate[5];

    TitlePhase phase = TitlePhase::StartWait;

};
