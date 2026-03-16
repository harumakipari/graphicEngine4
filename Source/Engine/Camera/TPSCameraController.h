#pragma once
#include "Components/Camera/CameraComponent.h"

class TPSCameraController
{
public:
    TPSCameraComponent* camera = nullptr;
    std::weak_ptr<SceneComponent> target;

    void Update(float dt)
    {
        auto t = target.lock();
        if (!t) return;

        using namespace DirectX;

        XMFLOAT3 targetPos = t->GetComponentLocation();

        XMVECTOR pivot =
            XMLoadFloat3(&targetPos) +
            XMVectorSet(0, 1.5f, 0, 0);

        static DirectX::XMFLOAT3 lastPos{};

        if (lastPos.x != targetPos.x ||
            lastPos.y != targetPos.y ||
            lastPos.z != targetPos.z)
        {
            Logger::Log("target moved");
        }

        lastPos = targetPos;

        float yaw = camera->yaw;
        float pitch = camera->pitch;

        XMVECTOR forward =
            XMVectorSet(
                sinf(yaw) * cosf(pitch),
                sinf(pitch),
                cosf(yaw) * cosf(pitch),
                0);

        XMVECTOR idealEye = pivot - forward * camera->distance;

        XMVECTOR currentEye = idealEye;

        XMFLOAT3 pos;
        XMStoreFloat3(&pos, currentEye); 

        //camera->SetWorldLocationDirect(pos)
        camera->GetOwner()->SetPosition(pos);
        XMFLOAT3 pivot3;
        XMStoreFloat3(&pivot3, pivot);

        camera->lookTarget = pivot3;
        camera->useLookTarget = true;
    }
};
