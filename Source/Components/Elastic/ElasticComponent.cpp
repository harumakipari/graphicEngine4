#include "pch.h"
#include "ElasticComponent.h"
#include "Core/Actor.h"
#include "Engine/Camera/CameraConstants.h"
#include "Engine/Debug/DebugDrawManager.h"
#include "Engine/Input/InputSystem.h"
#include "Game/Actors/Camera/Camera.h"
#include "Physics/CollisionFunction.h"

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
        /*maxAngleDegree*/ 100.0f, // 度以上は曲がらない
        /*modelHeight*/ modelHeight,
        /*stretchRate*/ 1.0f,
    };
    elasticBuildingCBuffer->data = elasticConstants;
}

void ElasticMeshComponent::Tick(float deltaTime)
{
    UpdatePushElastic(deltaTime);
    //UpdatePullElastic(deltaTime);
}

void ElasticMeshComponent::UpdatePullElastic(float deltaTime)
{
    DirectX::XMFLOAT3 position = owner_.lock()->GetPosition();


    // マウスカーソルを取得
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {// 左ボタンを押した瞬間
        dragStartMousePos = InputSystem::GetMousePosition();
        baseStretchRate = elasticConstants.stretchRate; // 今の伸び率を保存
        HitResultWithActor result;
        if (CollisionFunction::RaycastFromMouse(dragStartMousePos, result))
        {
            // ★ レイキャストして掴み点を1回だけ取得
            grabPointWorld = result.hitPoint;
            elasticConstants.grabPoint = { grabPointWorld.x,grabPointWorld.y,grabPointWorld.z };
            hasGrabPoint = true;
        }
    }
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::None) && hasGrabPoint)
    {// 左ボタンを押している間
        DirectX::XMFLOAT2 cursor = InputSystem::GetMousePosition();
        HitResultWithActor result;
        CollisionFunction::RaycastFromMouse(cursor, result);

        XMFLOAT3 currentWorldPos = result.hitPoint;

        // 引っ張りベクトル
        DirectX::XMVECTOR pullVec =
            XMLoadFloat3(&currentWorldPos) -
            XMLoadFloat3(&grabPointWorld);

        float pullLength = XMVectorGetX(XMVector3Length(pullVec));

        if (pullLength > 1e-4f)
        {
            DirectX::XMVECTOR pullDir = XMVector3Normalize(pullVec);

            UpdateBezierFromPull(pullDir, pullLength);
        }
    }
    else
    {// 左ボタンを押していない時
        hasGrabPoint = false;
        float midY = position.y + modelHeight * 0.5f;

        DirectX::XMFLOAT3 p = { elasticConstants.p3.x,elasticConstants.p3.y,elasticConstants.p3.z };
        float buildHeight = modelHeight;
        float targetX = position.x;
        float targetY = buildHeight;
        float targetZ = position.z;
        float gradX = p.x - targetX;// x - a
        float gradY = p.y - targetY;// x - a
        float gradZ = p.z - targetZ;// x - a

        elasticParameters.momentumX = elasticParameters.damping * elasticParameters.momentumX + gradX/*parmator*/;
        elasticParameters.momentumY = elasticParameters.damping * elasticParameters.momentumY + gradY/*parmator*/;
        elasticParameters.momentumZ = elasticParameters.damping * elasticParameters.momentumZ + gradZ/*parmator*/;
        p.x -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumX;
        p.y -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumY;
        p.z -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumZ;

        elasticConstants.p1 = { position.x,position.y,position.z,1.0f };
        elasticConstants.p2 = { position.x,midY,position.z ,1.0f };
        elasticConstants.p3 = { p.x,p.y,p.z,1.0f };
    }
    elasticConstants.maxAngleDegree = elasticParameters.maxAngleDegrees;
}

void ElasticMeshComponent::UpdateBezierFromPull(const DirectX::XMVECTOR& pullDir/*正規化して入ってくる*/, float pullLength)
{
    DirectX::XMFLOAT3 basePos = owner_.lock()->GetPosition();

    elasticConstants.pullDir = { DirectX::XMVectorGetX(pullDir),DirectX::XMVectorGetY(pullDir),DirectX::XMVectorGetZ(pullDir) };
    elasticConstants.pullLength = pullLength;

    float maxPull = maxPullLength;
    float clampedLen = std::min<float>(pullLength, maxPull);

    // p1 = 固定
    elasticConstants.p1 = { basePos.x, basePos.y, basePos.z, 1.0f };

    // p3 = 引っ張り先
    DirectX::XMVECTOR p3 =
        XMLoadFloat3(&basePos) +
        pullDir * clampedLen;

    XMStoreFloat4(&elasticConstants.p3, XMVectorSetW(p3, 1.0f));

    // p2 = 中間（柔らかさ）
    float softness = 0.3f; // 0.3～0.6
    DirectX::XMVECTOR p2 =
        XMLoadFloat3(&basePos) +
        pullDir * (clampedLen * softness);

    XMStoreFloat4(&elasticConstants.p2, XMVectorSetW(p2, 1.0f));

}


void ElasticMeshComponent::UpdatePushElastic(float deltaTime)
{
    DirectX::XMFLOAT3 position = owner_.lock()->GetPosition();
    // マウスカーソルを取得
    if (InputSystem::GetInputState("MouseLeft"))
    {// 左ボタンを押している間
        DirectX::XMFLOAT2 cursor = InputSystem::GetMousePosition();
        HitResultWithActor result;
        XMFLOAT3 intersectPos;
        XMFLOAT3 buildCurveDir;
        if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
        {
           DebugDrawManager::DrawSphere(result.hitPoint, 1.03f, { 1, 1, 0, 1 });

            XMFLOAT3 intersectNormal;
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
        DirectX::XMVECTOR WorldPostion;
        WorldPostion = DirectX::XMLoadFloat3(&buildTop);
        // スクリーン座標
        DirectX::XMFLOAT2 screenPosition = CollisionFunction::GetScreenPositionFromWorldPosition(buildTop);
        

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

        // 交点ベクトルを正規化＋距離クランプ
        XMFLOAT3 pos = position;
        DirectX::XMVECTOR basePos = DirectX::XMLoadFloat3(&pos);
        DirectX::XMVECTOR intersect = DirectX::XMLoadFloat3(&intersectPos);
        DirectX::XMVECTOR diff = intersect - basePos;
        diff = DirectX::XMVectorSetY(diff, 0.0f); // 高さは無視して水平方向だけのベクトルにする

        float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
        float maxDist = buildHeight + elasticParameters.maxDist; // 好みで調整
        float scale = (dist > maxDist) ? (maxDist / dist) : 1.0f;
        DirectX::XMVECTOR clampedDir = diff * scale;
        //DirectX::XMVECTOR clampedDir = diff * moveAmount; // mouse の変化量 を使う場合

        // p3 = 建物の上端＋方向ベクトル
        DirectX::XMFLOAT3 p3;
        DirectX::XMStoreFloat3(&p3, basePos + DirectX::XMVectorSet(0, buildHeight, 0, 0) + clampedDir);


        float midY = position.y + buildHeight * 0.5f;


        elasticConstants.p1 = { position.x,position.y,position.z,1.0f };
        elasticConstants.p2 = { position.x,midY,position.z ,1.0f };
        elasticConstants.p3 = { p3.x,p3.y,p3.z,1.0f };

    }
    else
    {// 左ボタンを押していない時

        float midY = position.y + modelHeight * 0.5f;

        DirectX::XMFLOAT3 p = { elasticConstants.p3.x,elasticConstants.p3.y,elasticConstants.p3.z };
        float targetX = position.x;
        float targetY = position.y + modelHeight;
        float targetZ = position.z;
        float gradX = p.x - targetX;// x - a
        float gradY = p.y - targetY;// x - a
        float gradZ = p.z - targetZ;// x - a

        elasticParameters.momentumX = elasticParameters.damping * elasticParameters.momentumX + gradX/*parmator*/;
        elasticParameters.momentumY = elasticParameters.damping * elasticParameters.momentumY + gradY/*parmator*/;
        elasticParameters.momentumZ = elasticParameters.damping * elasticParameters.momentumZ + gradZ/*parmator*/;
        p.x -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumX;
        p.y -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumY;
        p.z -= deltaTime * elasticParameters.stiffness * elasticParameters.momentumZ;

        elasticConstants.p1 = { position.x,position.y,position.z,1.0f };
        elasticConstants.p2 = { position.x,midY,position.z ,1.0f };
        elasticConstants.p3 = { p.x,p.y,p.z,1.0f };
        }
        elasticConstants.maxAngleDegree = elasticParameters.maxAngleDegrees;

        DebugDrawManager::DrawSphere({ elasticConstants.p1.x,elasticConstants.p1.y,elasticConstants.p1.z }, 0.03f, { 1, 0, 0, 1 });
        DebugDrawManager::DrawSphere({ elasticConstants.p2.x,elasticConstants.p2.y,elasticConstants.p2.z }, 0.03f, { 0, 1, 0, 1 });
        DebugDrawManager::DrawSphere({ elasticConstants.p3.x,elasticConstants.p3.y,elasticConstants.p3.z }, 0.03f, { 0, 0, 1, 1 });


#if 0
        std::wstring msg =
            L"elastic Constant: " + std::to_wstring(elasticConstants.stretchRate);

        OutputDebugStringW(msg.c_str());

#endif // 0
}


// サクランボのためにプリンの表面の位置を取得する関数
void  ElasticMeshComponent::GetSurfacePositionTangent(DirectX::XMFLOAT3& surfacePosition, DirectX::XMFLOAT3& tangent)
{
    XMVECTOR p2 = XMLoadFloat3((XMFLOAT3*)&elasticConstants.p2);
    XMVECTOR p3 = XMLoadFloat3((XMFLOAT3*)&elasticConstants.p3);

    XMVECTOR Tangent = XMVector3Normalize(2.0f * (p3 - p2)); // 接線ベクトル t=1.0fだから

    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR normal = XMVector3Normalize(XMVector3Cross(Tangent, XMVector3Cross(up, Tangent)));
    // プリンの半径
    XMVECTOR surfacePos = p3 + Tangent * cherryOffset;
    DirectX::XMStoreFloat3(&surfacePosition, surfacePos);

    // 表面
    DebugDrawManager::DrawSphere(surfacePosition, 0.03f, { 1,1,0,1 });
    // 接線
    XMVECTOR Debug = XMVectorAdd(p3, Tangent);
    XMFLOAT3 debugPos;
    XMStoreFloat3(&debugPos, Debug);
    DebugDrawManager::DrawLine({ elasticConstants.p3.x,elasticConstants.p3.y,elasticConstants.p3.z },
        debugPos, { 1,0,1,1 });

    XMStoreFloat3(&tangent, Tangent);
}


void ElasticMeshComponent::UpdateConstantBuffer(ID3D11DeviceContext* immediateContext) const
{
    elasticBuildingCBuffer->data = elasticConstants;
    elasticBuildingCBuffer->Activate(immediateContext, 6);
}
