#include "pch.h"
#include "PointLightComponent.h"

void PointLightComponent::Tick(float deltaTime)
{
#if _DEBUG
        DirectX::XMFLOAT3 pos = GetComponentLocation();
        DebugDrawManager::DrawSphere(
            pos,
            0.1f,
            DirectX::XMFLOAT4(color.x, color.y, color.z, 1.0f),
            0.0f
        );
#endif

}