#pragma once
#include <DirectXMath.h>

#include "Physics.h"
#include "Engine/Camera/CameraConstants.h"
#include "Engine/Camera/CameraManager.h"
#include "Game/Actors/Camera/Camera.h"
#include "Graphics/Core/Graphics.h"


namespace CollisionFunction
{
    inline bool RaycastFromMouse(const DirectX::XMFLOAT2& mouseCursor, HitResultWithActor& result, CollisionLayer collisionLayer = CollisionLayer::Everything)
    {
        float screenWidth = Graphics::GetScreenWidth();
        float screenHeight = Graphics::GetScreenHeight();

        // スクリーン座標の設定
        DirectX::XMVECTOR ScreenPosition, WorldPosition;
        DirectX::XMFLOAT3 screenPosition;
        screenPosition.x = static_cast<float>(mouseCursor.x);
        screenPosition.y = static_cast<float>(mouseCursor.y);
        screenPosition.z = 0.0f;
        ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);

        auto camera = CameraManager::GetCurrentCamera();
        ViewConstants data = camera->GetViewConstants();

        if (camera)
        {
            //各行列を取得
            DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&data.view);
            DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&data.projection);
            DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
            // スクリーン座標をワールド座標に変換し、レイの始点を求める
            WorldPosition = DirectX::XMVector3Unproject(ScreenPosition, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, Projection, View, World);

            DirectX::XMFLOAT3 rayStart;
            DirectX::XMStoreFloat3(&rayStart, WorldPosition);

            // スクリーン座標をワールド座標に変換し、レイの終点を求める
            screenPosition.x = static_cast<float>(mouseCursor.x);
            screenPosition.y = static_cast<float>(mouseCursor.y);
            screenPosition.z = 1.0f;
            ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);
            WorldPosition = DirectX::XMVector3Unproject(
                ScreenPosition, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, Projection, View, World
            );
            DirectX::XMFLOAT3 rayEnd;
            DirectX::XMStoreFloat3(&rayEnd, WorldPosition);
            DirectX::XMVECTOR RayDir = DirectX::XMVectorSubtract(XMLoadFloat3(&rayEnd), XMLoadFloat3(&rayStart));
            float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(RayDir));
            RayDir = DirectX::XMVector3Normalize(RayDir);
            DirectX::XMFLOAT3 rayDir;
            DirectX::XMStoreFloat3(&rayDir, RayDir);
            
            if (Physics::Instance().SphereCast(rayStart, rayDir, FLT_MAX, 0.001f, result, static_cast<uint32_t>(collisionLayer)))
            {
                return true;
            }
            else
            {
                return false;
            }

        }
        return false;
    }

    inline XMFLOAT2 GetScreenPositionFromWorldPosition(const XMFLOAT3& worldPosition)
    {
        float screenWidth = Graphics::GetScreenWidth();
        float screenHeight = Graphics::GetScreenHeight();
        DirectX::XMVECTOR WorldPosition = DirectX::XMLoadFloat3(&worldPosition);
        auto camera = CameraManager::GetCurrentCamera();
        ViewConstants data = camera->GetViewConstants();
        //各行列を取得
        DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&data.view);
        DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&data.projection);
        DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
        // ワールド座標をスクリーン座標に変換
        DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
            WorldPosition, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, Projection, View, World
        );
        DirectX::XMFLOAT3 screenPos;
        DirectX::XMStoreFloat3(&screenPos, ScreenPosition);
        return XMFLOAT2(screenPos.x, screenPos.y);
    }
}
