#include "pch.h"
#include "ElasticComponent.h"
#include "Core/Actor.h"
#include "Engine/Camera/CameraConstants.h"
#include "Engine/Input/InputSystem.h"
#include "Game/Actors/Camera/Camera.h"

void ElasticMeshComponent::Initialize()
{
    DirectX::XMFLOAT3 position = owner_.lock()->GetPosition();
    modelHeight = model->GetModelSize().y;
    float midY = position.y + modelHeight * 0.5f;
    // 定数バッファの作成
    elasticBuildingCBuffer = std::make_unique<ConstantBuffer<ElasticConstants>>(Graphics::GetDevice());
    elasticConstants =
    {
        /*p1*/ DirectX::XMFLOAT4(position.x, position.y, position.z, 1.0f),
        /*p2*/ DirectX::XMFLOAT4(position.x,position.y, position.z, 1.0f),
        /*p3*/ DirectX::XMFLOAT4(position.x, position.y + modelHeight, position.z, 1.0f),
        /*buildProgress*/ 0.0f,
        /*modelHeight*/ modelHeight
    };
    elasticBuildingCBuffer->data = elasticConstants;
}

void ElasticMeshComponent::Tick(float deltaTime)
{
    DirectX::XMFLOAT3 position = owner_.lock()->GetPosition();
    // マウスカーソルを取得
    if (InputSystem::GetInputState("MouseLeft"))
    {// 左ボタンを押している間
        DirectX::XMFLOAT2 cursor = InputSystem::GetMousePosition();
        float screenWidth = Graphics::GetScreenWidth();
        float screenHeight = Graphics::GetScreenHeight();

        // スクリーン座標の設定
        DirectX::XMVECTOR ScreenPosition, WorldPosition;
        DirectX::XMFLOAT3 screenPosition;
        screenPosition.x = static_cast<float>(cursor.x);
        screenPosition.y = static_cast<float>(cursor.y);
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
            screenPosition.x = static_cast<float>(cursor.x);
            screenPosition.y = static_cast<float>(cursor.y);
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
            XMFLOAT3 intersectPos, intersectNormal;
            std::string intersectionMesh, intersectionMaterial;
            DirectX::XMFLOAT3 buildCurveDir;

            HitResultWithActor result;
            if (Physics::Instance().SphereCast(rayStart, rayDir, FLT_MAX, 0.001f, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
            {
                if (auto stage = dynamic_cast<FightStage*>(result.actor))
                {
                    intersectPos = result.hitPoint;
                    intersectNormal = result.normal;
                }
                else
                {
                    intersectPos = { 0.0f,0.0f,0.0f };
                    intersectNormal = { 0.0f,0.0f,0.0f };
                }
                buildCurveDir = { intersectPos.x - position.x,0.0f,intersectPos.z - position.z };
            }
            else
            {
                intersectPos = { 0.0f,0.0f,0.0f };
                buildCurveDir = { 0.0f,0.0f,0.0f };
            }

            // これでマウスのpositionによって、建物を曲げる方向を見つける
            XMVECTOR BuildCurveDir = XMLoadFloat3(&buildCurveDir);
            BuildCurveDir = XMVector3Normalize(BuildCurveDir);
            float buildHeight = modelHeight;
            BuildCurveDir = XMVectorScale(BuildCurveDir, buildHeight);
            XMStoreFloat3(&buildCurveDir, BuildCurveDir);

            //　建物の一番上からのマウスの移動量によって、建物を曲げる量を決める
            // ワールド座標からスクリーン座標へ変換
            DirectX::XMFLOAT3 buildTop = { position.x,position.y + buildHeight,position.z };
            DirectX::XMVECTOR WorldPostion, ScreenPosition;
            WorldPostion = DirectX::XMLoadFloat3(&buildTop);
            ScreenPosition = DirectX::XMVector3Project(WorldPostion, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, Projection, View, World);
            // スクリーン座標
            DirectX::XMFLOAT2 screenPosition;
            DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

            // とりあえずｘだけの移動量
            float moveAmount = cursor.x - screenPosition.x;


            // 建物中心と交点の差ベクトル (水平)
            DirectX::XMFLOAT3 dirXZ = {
                intersectPos.x - position.x,
                0.0f,
                intersectPos.z - position.z
            };
            float angle = 0.0f;
            DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&dirXZ);
            if (DirectX::XMVector3LengthSq(dirVec).m128_f32[0] < 1e-6f)
            {
                // マウスが真上＝曲げ方向なし
                angle = 0.0f;
            }
            else
            {
                // atan2(Z,X) で角度をラジアン取得
                angle = atan2f(dirXZ.x, dirXZ.z);
            }

            float midY = position.y + buildHeight * 0.5f;
            // 交点ベクトルを正規化＋距離クランプ
            XMFLOAT3 pos = position;
            DirectX::XMVECTOR basePos = DirectX::XMLoadFloat3(&pos);
            DirectX::XMVECTOR intersect = DirectX::XMLoadFloat3(&intersectPos);
            DirectX::XMVECTOR diff = intersect - basePos;
            diff = DirectX::XMVectorSetY(diff, 0.0f); // 高さは無視して水平方向だけのベクトルにする

            float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
            float maxDist = buildHeight; // 好みで調整
            float scale = (dist > maxDist) ? (maxDist / dist) : 1.0f;
            DirectX::XMVECTOR clampedDir = diff * scale;
            //DirectX::XMVECTOR clampedDir = diff * moveAmount; // mouse の変化量 を使う場合

            // p3 = 建物の上端＋方向ベクトル
            DirectX::XMFLOAT3 p3;
            DirectX::XMStoreFloat3(&p3, basePos + DirectX::XMVectorSet(0, buildHeight, 0, 0) + clampedDir);
            elasticConstants.p1 = { position.x,position.y,position.z,1.0f };
            elasticConstants.p2 = { position.x,midY,position.z ,1.0f };
            elasticConstants.p3 = { p3.x,p3.y,p3.z,1.0f };
        }
    }
    else
    {// 左ボタンを押していない時

        static float momentumX = -0.8f;  // 慣性バッファ
        static float momentumY = -0.8f;  // 慣性バッファ
        static float momentumZ = -0.8f;  // 慣性バッファ
        float speed = 4.0f;     // ボールの硬さ
        float damping = 0.95f;   // 減衰率
        float midY = position.y + modelHeight * 0.5f;

        DirectX::XMFLOAT3 p = { elasticConstants.p3.x,elasticConstants.p3.y,elasticConstants.p3.z };
        float buildHeight = modelHeight;
        float targetX = position.x;
        float targetY = buildHeight;
        float targetZ = position.z;
        float gradX = p.x - targetX;// x - a
        float gradY = p.y - targetY;// x - a
        float gradZ = p.z - targetZ;// x - a

        momentumX = damping * momentumX + gradX/*parmator*/;
        momentumY = damping * momentumY + gradY/*parmator*/;
        momentumZ = damping * momentumZ + gradZ/*parmator*/;
        p.x -= deltaTime * speed * momentumX;
        p.y -= deltaTime * speed * momentumY;
        p.z -= deltaTime * speed * momentumZ;

        elasticConstants.p1 = { position.x,position.y,position.z,1.0f };
        elasticConstants.p2 = { position.x,midY,position.z ,1.0f };
        elasticConstants.p3 = { p.x,p.y,p.z,1.0f };
    }
}

void ElasticMeshComponent::UpdateConstantBuffer(ID3D11DeviceContext* immediateContext) const 
{
    elasticBuildingCBuffer->data = elasticConstants;
    elasticBuildingCBuffer->Activate(immediateContext, 6);
}
