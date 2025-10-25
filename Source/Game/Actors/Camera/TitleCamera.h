#ifndef TITLE_CAMERA_H
#define TITLE_CAMERA_H

#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Components/Camera/CameraComponent.h"
#include "Game/Actors/Camera/Camera.h"

class TitleCamera :public Camera
{
public:
    //引数付きコンストラクタ
    TitleCamera(std::string actorName) :Camera(actorName) {}

    virtual ~TitleCamera() = default;
    //std::shared_ptr<CameraComponent> mainCameraComponent;


    void Initialize()override
    {
        mainCameraComponent = this->NewSceneComponent<class CameraComponent>("mainCamera");
        mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(35), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
        SetPosition(DirectX::XMFLOAT3(-0.13f, 1.2f, -4.3f));
        SetEulerRotation(DirectX::XMFLOAT3(1.4f, 20.6f, 0.0f));
    };

    //更新処理
    void Update(float deltaTime)override
    {
        using namespace DirectX;
#if 0

        XMFLOAT3 forward = enemy->GetForward();
        DirectX::XMFLOAT3 focus = enemy->bossJointComponent->GetComponentWorldTransform().GetLocation();
        XMVECTOR Focus = XMLoadFloat3(&focus);

        XMVECTOR Forward = XMVector3Normalize(XMLoadFloat3(&forward));
        XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
        XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Forward, Up));

        XMVECTOR Eye = Focus + (Right * -easeX) + (Forward * easeZ);
        XMFLOAT3 eye;
        XMStoreFloat3(&eye, Eye);

        if (abs(focus.x - eye.x) < 0.001f && abs(focus.z - eye.z) < 0.001f)
        {
            Eye += Forward * distance;
            XMStoreFloat3(&eye, Eye);
        }
        eye.y = easeY;

        mainCameraComponent->customTarget = true;
        mainCameraComponent->_target = focus;
        SetPosition(eye);
#else
        XMFLOAT3 focus = target;   

        XMVECTOR Focus = XMLoadFloat3(&focus);
        XMVECTOR dir = XMVector3Normalize(XMVectorSet(0.3f, 5.2f, -1.0f, 0)); // 好きな方向
        XMVECTOR eyePos = Focus - dir * distance;

        XMFLOAT3 eye;
        XMStoreFloat3(&eye, eyePos);

        // カメラ本体の Transform を更新（必要なら）
        SetPosition(eye);

        // ビュー行列を計算
        mainCameraComponent->LookAt(eye, focus, XMFLOAT3(0, 1, 0));

#endif // 0
    }

    void OnClick()
    {
        isClickButton = true;
    }

    void SetTarget(DirectX::XMFLOAT3 target)
    {
        this->target = target;
    }

    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI
        ImGui::DragFloat3("offset", &offset.x, 0.3f);
        ImGui::DragFloat3("offset2", &offset2.x, 0.3f);
        ImGui::DragFloat3("cameraTarget", &target.x, 0.3f);
        ImGui::DragFloat("distance", &distance, 0.3f);
#endif
    }

private:
    DirectX::XMFLOAT3 target = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 offset = { 0.0f,1.2f,0.0f };
    DirectX::XMFLOAT3 offset2 = { 0.0f,1.2f,0.0f };
    float distance = 10.0f;
    bool isClickButton = false;
};

#endif // !TITLE_CAMERA_H
