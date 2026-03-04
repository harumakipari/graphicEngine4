#include "pch.h"
#include "PointLightComponent.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneBase.h"

void PointLightComponent::Initialize()
{
    enable = true;
}

void PointLightComponent::Tick(float deltaTime)
{
#if _DEBUG
        DirectX::XMFLOAT3 pos = GetComponentLocation();
        //DebugDrawManager::DrawSphere(
        //    pos,
        //    0.1f,
        //    DirectX::XMFLOAT4(sharedParam->color.x, sharedParam->color.y, sharedParam->color.z, 1.0f),
        //    0.0f
        //);

        //DebugDrawManager::DrawCylinder(
        //    pos,
        //    sharedParam->range, 0.5f,
        //    DirectX::XMFLOAT4(sharedParam->color.x, sharedParam->color.y, sharedParam->color.z, 1.0f),
        //    0.0f
        //);

#endif

}