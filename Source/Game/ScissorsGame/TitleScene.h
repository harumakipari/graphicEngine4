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


class TitleCameraTargetActor;
class TitleBookActor;

class TitleScene : public SceneBase
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

    // タイトルシーンへ
    void StartToTitle();

    // セレクトシーンへ
    void StartToSelect();

    //シーンの自動登録
    static inline Scene::Autoenrollment<TitleScene> _autoenrollment;

    // セレクトシーンから開始する
    void StartSelectScene();


private:
    TPSCameraComponent* mainCameraComponent = nullptr;
    std::shared_ptr<TitleBookActor> bookActor; // 本のアクター
    std::shared_ptr<TitleCamera> mainCameraActor; // メインカメラのアクター
    std::shared_ptr<TitleCameraTargetActor> cameraTargetActor; // カメラターゲットのアクター

    float toSelectInterval = 2.5f; // セレクトシーンまでにかかる時間
    float toTitleInterval = 3.5f; // タイトルシーンまでにかかる時間

    std::shared_ptr<UIImageComponent> mouseCursorPar;   // マウスパー
    std::shared_ptr<UIImageComponent> mouseCursorGrab;  // マウス掴み
    std::shared_ptr<UIImageComponent> mouseCursorPause; // マウス　ポーズ

};
