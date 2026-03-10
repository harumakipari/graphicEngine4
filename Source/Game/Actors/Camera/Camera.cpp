#include "pch.h"
#include "Camera.h"

#include "Components/Controller/ControllerComponent.h"
#include "Engine/Scene/SceneBase.h"

void Camera::Initialize(const Transform& transform)
{
    mainCameraComponent = this->AddComponent<class TPSCameraComponent>("mainCamera");
    mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(35), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
    //mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(45), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 1.1f, 100.0f);

#if 0
    auto scene = dynamic_cast<SceneBase*>(Scene::GetCurrentScene());
    // ポイントライトコンポーネントを追加
    auto pointLightComponent = this->AddComponent<PointLightComponent>("pointLightComponent", "mainCamera");
    pointLightComponent->SetRelativeLocationDirect({ 0.0f, 1.5f, 0.6f });
    auto lightManager = scene->GetLightManager();
    // ライトの名前からライトマネージャーの共有ライトを取得して設定
    if (auto shared = lightManager->FindSharedLight("CameraPointLight"))
    {
        pointLightComponent->SetSharedParam(shared);
    }


#endif // 0
}


void MainCamera::Update(float deltaTime)
{

    if (auto target = mainCameraComponent->target.lock())
    {
        auto actor = target->GetOwner();
        
    }




    // プレイヤーの移動方向を取得
    XMFLOAT3 moveDir = {};

    if (auto target = mainCameraComponent->target.lock())
    {
        auto actor = target->GetOwner();

        if (auto movement = actor->GetComponent<CharacterMovementComponent>())
        {
            moveDir = movement->GetVelocity();
        }
    }

    // 右スティック
    XMFLOAT2 rightStick =
        InputSystem::GetRightStick();

    // AutoFollow 呼び出し
    //static_cast<TPSCameraComponent*>(mainCameraComponent.get())->AutoFollow(moveDir, rightStick, deltaTime);
}

