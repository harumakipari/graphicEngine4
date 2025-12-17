#include "pch.h"
#include "SampleScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"
#include "Game/Actors/Enemy/Boss/BossEnemy.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/Cloth.h"
#include "Game/SofyBody/MassPoint.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/TitleUIFactory.h"

#include "Physics/Physics.h"
#include "Game/Actors/Stage/FightStage.h"

#include "Graphics/PostProcess/BloomEffect.h"
#include "Physics/CollisionSystem.h"


bool SampleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();
    EventSystem::Initialize();//追加 UI
    return true;
}

void SampleScene::Start()
{
}

void SampleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);

    Physics::Instance().Update(deltaTime);
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加

    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();


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
            auto stage = GetActorManager()->GetActorByName("title");
            XMFLOAT3 intersectPos, intersectNormal;
            std::string intersectionMesh, intersectionMaterial;
            DirectX::XMFLOAT3 buildCurveDir;
            // レイキャストテスト
            HitResult hit;
            if (Physics::Instance().RayCast(
                rayStart, rayDir,
                FLT_MAX,
                hit))
            {
                Graphics::GetShapeRenderer()->DrawSphere(hit.position, 0.1f, { 1, 0, 0, 1 });
            }


            //RaycastHit2 result;
            //if (Physics::Instance().SphereCast(rayStart, rayDir, FLT_MAX, 1.0f, result))   // wantHitRayer)
            //{
            //    if (auto stage = dynamic_cast<Stage*>(result.actor))
            //    {
            //        intersectPos = result.hitPoint;
            //        intersectNormal = result.normal;
            //    }
            //    else
            //    {
            //        intersectPos = { 0.0f,0.0f,0.0f };
            //        intersectNormal = { 0.0f,0.0f,0.0f };
            //    }
            //    buildCurveDir = { intersectPos.x - build->GetPosition().x,0.0f,intersectPos.z - build->GetPosition().z };
            //}
            //else
            //{
            //    intersectPos = { 0.0f,0.0f,0.0f };
            //    buildCurveDir = { 0.0f,0.0f,0.0f };
            //}

        //    // これでマウスのpositionによって、建物を曲げる方向を見つける
        //    XMVECTOR BuildCurveDir = XMLoadFloat3(&buildCurveDir);
        //    BuildCurveDir = XMVector3Normalize(BuildCurveDir);
        //    float buildHeight = 5.0f;
        //    BuildCurveDir = XMVectorScale(BuildCurveDir, buildHeight);
        //    XMStoreFloat3(&buildCurveDir, BuildCurveDir);

        //    //　建物の一番上からのマウスの移動量によって、建物を曲げる量を決める
        //    // ワールド座標からスクリーン座標へ変換
        //    DirectX::XMFLOAT3 buildTop = { build->GetPosition().x,build->GetPosition().y + buildHeight,build->GetPosition().z };
        //    DirectX::XMVECTOR WorldPostion, ScreenPosition;
        //    WorldPostion = DirectX::XMLoadFloat3(&buildTop);
        //    ScreenPosition = DirectX::XMVector3Project(WorldPostion, 0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f, Projection, View, World);
        //    // スクリーン座標
        //    DirectX::XMFLOAT2 screenPosition;
        //    DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

        //    // とりあえずｘだけの移動量
        //    float moveAmount = cursor.x - screenPosition.x;


        //    // 建物中心と交点の差ベクトル (水平)
        //    DirectX::XMFLOAT3 dirXZ = {
        //        intersectPos.x - build->GetPosition().x,
        //        0.0f,
        //        intersectPos.z - build->GetPosition().z
        //    };
        //    float angle = 0.0f;
        //    DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&dirXZ);
        //    if (DirectX::XMVector3LengthSq(dirVec).m128_f32[0] < 1e-6f)
        //    {
        //        // マウスが真上＝曲げ方向なし
        //        angle = 0.0f;
        //    }
        //    else
        //    {
        //        // atan2(Z,X) で角度をラジアン取得
        //        angle = atan2f(dirXZ.x, dirXZ.z);
        //    }

        //    float midY = build->GetPosition().y + buildHeight * 0.5f;
        //    // 交点ベクトルを正規化＋距離クランプ
        //    XMFLOAT3 pos = build->GetPosition();
        //    DirectX::XMVECTOR basePos = DirectX::XMLoadFloat3(&pos);
        //    DirectX::XMVECTOR intersect = DirectX::XMLoadFloat3(&intersectPos);
        //    DirectX::XMVECTOR diff = intersect - basePos;
        //    diff = DirectX::XMVectorSetY(diff, 0.0f); // 高さは無視して水平方向だけのベクトルにする

        //    float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
        //    float maxDist = buildHeight; // 好みで調整
        //    float scale = (dist > maxDist) ? (maxDist / dist) : 1.0f;
        //    DirectX::XMVECTOR clampedDir = diff * scale;
        //    //DirectX::XMVECTOR clampedDir = diff * moveAmount; // mouse の変化量 を使う場合

        //    // p3 = 建物の上端＋方向ベクトル
        //    DirectX::XMFLOAT3 p3;
        //    DirectX::XMStoreFloat3(&p3, basePos + DirectX::XMVectorSet(0, buildHeight, 0, 0) + clampedDir);
        //    if (auto building = std::dynamic_pointer_cast<ElasticBuilding>(build))
        //    {
        //        building->p1 = build->GetPosition();
        //        building->p2 = { build->GetPosition().x,midY,build->GetPosition().z };
        //        building->p3 = p3;
        //    }
        //}
        }
        else
        {// 左ボタンを押していない時
            //if (auto building = std::dynamic_pointer_cast<ElasticBuilding>(build))
            //{
            //    static float momentumX = -0.8f;  // 慣性バッファ
            //    static float momentumY = -0.8f;  // 慣性バッファ
            //    static float momentumZ = -0.8f;  // 慣性バッファ
            //    float speed = 4.0f;     // ボールの硬さ
            //    float damping = 0.95f;   // 減衰率

            //    DirectX::XMFLOAT3 p = building->p3;
            //    float buildHeight = 5.0f;
            //    float targetX = build->GetPosition().x;
            //    float targetY = buildHeight;
            //    float targetZ = build->GetPosition().z;
            //    float gradX = p.x - targetX;// x - a
            //    float gradY = p.y - targetY;// x - a
            //    float gradZ = p.z - targetZ;// x - a

            //    momentumX = damping * momentumX + gradX/*parmator*/;
            //    momentumY = damping * momentumY + gradY/*parmator*/;
            //    momentumZ = damping * momentumZ + gradZ/*parmator*/;
            //    p.x -= deltaTime * speed * momentumX;
            //    p.y -= deltaTime * speed * momentumY;
            //    p.z -= deltaTime * speed * momentumZ;

            //    building->p1 = build->GetPosition();
            //    building->p2 = { build->GetPosition().x,buildHeight,build->GetPosition().z };
            //    building->p3 = p;
            //    //building->p3 = { build->GetPosition().x,buildHeight,build->GetPosition().z };
            //}
        }

#ifdef _DEBUG
        if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
        {
            const char* types[] = { "0", "1" };
            Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
        }
#endif // !_DEBUG
    }
}

void SampleScene::SetUpActors()
{
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto player = this->GetActorManager()->CreateAndRegisterActorWithTransform<Player>("player", playerTr);
    mainCameraComponent->target = (player->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(19.0f);
    //mainCameraComponent->followTarget = (titlePlayer->GetRootComponent());
    //mainCameraComponent->lookAtTarget = (titlePlayer->GetRootComponent());

    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<FightStage>("stage", stageTr);

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    Transform buildTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto building = this->GetActorManager()->CreateAndRegisterActorWithTransform<ElasticBuilding>("building", buildTr);

#if 1
    CameraManager::SetGameCamera(mainCameraActor.get());
#else
    CameraManager::SetGameCamera(debugCameraActor.get());
#endif // 0
    //stageCollisionMesh = std::make_shared<CollisionMesh>(Graphics::GetDevice(), "./Data/Models/Stage/stage.gltf", true);

    Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<BossEnemy>("enemy", enemyTr);


    CameraManager::SetDebugCamera(debugCameraActor);
}

bool SampleScene::Uninitialize(ID3D11Device* device)
{
    Physics::Instance().Finalize();
    return true;
}

void SampleScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    SceneBase::Render(immediateContext, deltaTime);
}

void SampleScene::DrawGui()
{
    SceneBase::DrawGui();
}
