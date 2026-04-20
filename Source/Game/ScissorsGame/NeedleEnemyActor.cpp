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
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // emissionPowerの値を大きくして、自己発光の強さを上げてみる
    skeletalMeshComponent->overrideDeferredPipelineName = "deferredFightStage";
    skeletalMeshComponent->plusAlphaCBuffer->data.brightness = 5.0f;
    skeletalMeshComponent->plusAlphaCBuffer->data.saturation = 1.4f;

    // 当たり判定
    {
        std::shared_ptr<SphereComponent> sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(mass);
        sphereComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereComponent->SetLayer(CollisionLayer::Enemy);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetCollisionOffsetY(height * 0.5f);
        sphereComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };

    // 星のエフェクトを追加
    starEffectComponent = this->AddComponent<ParticleComponent>("starEffect", parentName);
    starEffectComponent->Load("./Data/Effect/Files/ScissorsGameStarEffect.json");


    auto uiManager = GetOwnerScene()->GetUIManager();
    auto ring = std::make_shared<UIRingEffect>("./Data/Textures/ScissorsUI/ring.png");
    ring->SetWorldPosition({ 500, 300 });
    ring->SetSize({ 100,100 });
    uiManager->Add(ring);
    for (int i = 0; i < 8; i++)
    {
        auto star = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png", DirectX::XMFLOAT2{ 500, 300 });
        //star->SetWorldPosition({ 500, 300 });
        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }

    // 死亡した時に呼ばれる関数
    onDeath = [this]()
        {
            BreakAllWalls();
        };

}

void NeedleEnemyActor::Update(float deltaTime)
{
    MoveLinear(deltaTime);

    DirectX::XMFLOAT3 pos = GetPosition();

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

// 壁を全て壊す
void NeedleEnemyActor::BreakAllWalls()
{
    for (auto& wall:walls)
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

