#include "pch.h"

#include "ScissorsPlayer.h"
#include "Engine/Scene/SceneBase.h"
#include "ScissorsActor.h"

void ScissorsPlayer::Initialize(const Transform& transform)
{
    int maxHp = 100;
    hp = maxHp;

    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Player/player.gltf", false, true);

    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    controller->AddAnimation("Attack", 0);
    controller->AddAnimation("Walk", 1);
    controller->AddAnimation("Death", 2);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    PlayAnimation("Walk");

#if 0
    // 当たり判定
    {
        std::shared_ptr<CapsuleComponent> capsuleComponent = this->AddComponent<class CapsuleComponent>("capsuleComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        height = size.y;
        radius = size.x * 0.5f;
        mass = 60.0f;
        capsuleComponent->SetRadiusAndHeight(radius, height);
        capsuleComponent->SetMass(mass);
        capsuleComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        capsuleComponent->SetLayer(CollisionLayer::Enemy);
        capsuleComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetCollisionOffsetY(height * 0.5f);
        capsuleComponent->SetIsVisibleDebugBox(false);
        capsuleComponent->Initialize();
    }
#endif // 0

    // 入力用のコンポーネントを追加
    inputComponent = this->AddComponent<class InputComponent>("inputComponent", parentName);

    // 移動用コンポーネントを追加
    characterMovementComponent = this->AddComponent<CharacterMovementComponent>("movementComponent", parentName);
    characterMovementComponent->SetUseGravity(false);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);
    rotationComponent->SetRotateTime(0.1f); // 回転を速くする

    // 
    for (int i = 0; i < 2; i++)
    {
        Transform scissorsTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.5f,0.5f,0.5f });
        auto scissors = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsActor>("scissors", scissorsTr);
        scissors->SetOwnerPlayer(this);
        scissors->PickUp(); // 最初は持ってる状態

        equippedScissors.push_back(scissors);
    }

    scissorsCount = 2;

}

void ScissorsPlayer::Update(float deltaTime)
{
    Character::Update(deltaTime);

#if 1 // 入力に基づいて移動と回転を更新
    auto intent = inputComponent->GetIntent();
    DirectX::XMFLOAT3 moveDir = { 0,0,0 };

    // 左スティック入力
    float stickX = intent.leftMove.x;
    float stickZ = intent.leftMove.z;

    moveDir.x = stickX;
    moveDir.z = stickZ;

    characterMovementComponent->SetMoveDirection(moveDir);
    rotationComponent->SetDirection(moveDir);

#endif // 0

    if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Trigger))
    {
        if (scissorsCount == 2)
        {
            // 1本置く
            Logger::Log(U8("ハサミを置く"));
            DropOne();
        }
        else // scissorsCount == 1
        {
            if (auto scissors = FindNearestDroppedScissors())
            {// 近くに落ちているハサミがあるなら
                // 拾って2本になる
                Logger::Log(U8("ハサミを拾う"));
                PickUpNearest();
            }
            else
            {
                //  引き寄せ
                Logger::Log(U8("ハサミを引き寄せる"));
                PullNearest();
            }
        }
    }
    if (InputSystem::GetInputState("ScissorsAttack", InputStateMask::Trigger))
    {
        for (auto& w : equippedScissors)
        {
            auto s = w.lock();
            if (!s) continue;

            s->StartAttack();
            Logger::Log(U8("攻撃をする"));
        }
    }

}

// ハサミを落とす
void ScissorsPlayer::DropOne()
{
    if (equippedScissors.size() <= 1) return;

    auto scissors = equippedScissors.back();
    auto scissorsPtr = scissors.lock();

    if (!scissorsPtr)
    {
        Logger::Warning(U8("落とすハサミがnullptrです"));
    }

    equippedScissors.pop_back();

    scissorsPtr->Drop(GetPosition());

    droppedScissors.push_back(scissors);

    scissorsCount--;
}

// ハサミを拾う
void ScissorsPlayer::PickUpNearest()
{
    if (droppedScissors.empty()) return;

    auto scissors = droppedScissors.back(); // とりあえず一番近い扱い
    auto scissorsPtr = scissors.lock();

    if (!scissorsPtr)
    {
        Logger::Warning(U8("拾うハサミがnullptrです"));
    }

    scissorsPtr->PickUp();

    equippedScissors.push_back(scissors);
    droppedScissors.pop_back();

    scissorsCount++;
}

// ハサミを引き寄せる
void ScissorsPlayer::PullNearest()
{
    if (droppedScissors.empty()) return;

    auto scissors = droppedScissors.back();
    auto scissorsPtr = scissors.lock();

    if (!scissorsPtr)
    {
        Logger::Warning(U8("引き寄せるハサミがnullptrです"));
    }

    scissorsPtr->StartPull(GetPosition());
}

ScissorsActor* ScissorsPlayer::FindNearestDroppedScissors()
{
    float minDist = FLT_MAX;
    ScissorsActor* nearest = nullptr;

    auto playerPos = GetPosition();

    for (auto& w : droppedScissors)
    {
        auto s = w.lock();
        if (!s) continue;

        auto pos = s->GetPosition();
        float dist = MathHelper::Distance(playerPos, pos);

        if (dist < minDist)
        {
            minDist = dist;
            nearest = s.get();
        }
    }

    // 距離制限つけると良い
    if (minDist < 3.0f)
        return nearest;

    return nullptr;
}