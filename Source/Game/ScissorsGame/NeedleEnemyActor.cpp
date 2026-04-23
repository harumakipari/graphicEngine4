#include "pch.h"
#include "NeedleEnemyActor.h"

#include "RibbonWallActor.h"
#include "Engine/Scene/Scene.h"
#include "UI/Widgets/Widget.h"


void NeedleEnemyActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/NeedleEnemy.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

    // 当たり判定
    {
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(mass);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Enemy);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 最初の位置に待ち針を生成する
    Transform pinTr{ startPosition,{0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} };
    needlePinActor= GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<NeedlePinActor>("needlePin", pinTr);

    // 倒したときのスコア
    scoreData = { 100,0 };

    // 星のエフェクトを追加
    starEffectComponent = this->AddComponent<ParticleComponent>("starEffect", parentName);
    starEffectComponent->Load("./Data/Effect/Files/ScissorsGameStarEffect.json");

    // 死亡した時に呼ばれる関数
    onDeath = [this]()
        {
            BreakAllWalls();
        };


    // 最初に壁を生成する
    SpawnWall(transform.GetLocation());
}

void NeedleEnemyActor::Update(float deltaTime)
{
    ScissorsGameEnemyBase::Update(deltaTime);

#if 1
    if (isStopped)
    {// 止まっていたら
        return;
    }
#endif // 0

    DirectX::XMFLOAT3 pos = GetPosition();

    float speed = 3.0f;
    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    SetPosition(pos);

    // ステージの端かどうかを確認する
    CheckStageEdge();

    if (rotationComponent)
    {
        rotationComponent->SetDirection(moveDirection);
    }

    if (hp <= 0) // 死んでいる場合は壁を生成しない
        return;

    // 壁生成チェック
    float dx = pos.x - lastDropPos.x;
    float dz = pos.z - lastDropPos.z;
    float dist = sqrt(dx * dx + dz * dz);

    if (dist > dropDistance)
    {
        lastDropPos = pos;
        SpawnWall(pos);
    }
}

// 終了時の処理
void NeedleEnemyActor::Finalize()
{
    // 待ち針アクターを削除
    needlePinActor->MarkPendingKill();
}

// 壁を全て壊す
void NeedleEnemyActor::BreakAllWalls()
{
    for (auto& wall : walls)
    {
        wall->Break();
    }
}

// 壁を生成する
void NeedleEnemyActor::SpawnWall(const DirectX::XMFLOAT3& pos)
{
    auto scene = GetOwnerScene();

    // 壁を生成
    Transform tr{ pos,DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f },{ 0.5f, 1.0f, 0.5f } };
    auto wall = scene->GetActorManager()->CreateAndRegisterActorWithTransform<RibbonWallActor>("RibbonWall", tr);

    // 壁を所有する敵を設定する
    wall->ownerEnemy = std::static_pointer_cast<NeedleEnemyActor>(shared_from_this());
    walls.push_back(wall);
}

// ステージ端かどうか
void NeedleEnemyActor::CheckStageEdge()
{
    auto pos = GetPosition();

    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;

    if (pos.x < stageMinX || pos.x > stageMaxX ||
        pos.z < stageMinZ || pos.z > stageMaxZ)
    {
        isStopped = true;
    }
}


void NeedlePinActor::Initialize(const Transform& transform)
{
    std::string parentName = "needleModel";
    // 待ち針のモデルを追加
    needlePinComponent = AddComponent<SkeletalMeshComponent>(parentName);
    needlePinComponent->SetModel("./Data/TeamModels/Enemy/NeedlePin.glb", false, true);
}
