#include "pch.h"
#include "BossEnemy.h"
#include "BossState.h"

#include "Components/Controller/ControllerComponent.h"
#include "Engine/Scene/Scene.h"

void BossEnemy::Initialize(const Transform& transform)
{
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent = this->AddComponent<class SkeletalMeshComponent>("skeletalComponent");
    //skeletalMeshComponent->SetModel("./Data/Models/Characters/Savarog/Idle.gltf");
    skeletalMeshComponent->SetModel("./Data/Models/Characters/SevarogBloodred/Idle.gltf");
    skeletalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
    const std::vector<std::string> animationFilenames =
    {
        "./Data/Models/Characters/Savarog/Jog_Fwd.glb",
        "./Data/Models/Characters/Savarog/Jog_Left.glb",
        "./Data/Models/Characters/Savarog/Jog_Right.glb",
        "./Data/Models/Characters/Savarog/LevelStart.glb",
        "./Data/Models/Characters/Savarog/Recall.glb",
        "./Data/Models/Characters/Savarog/Emote_Pull_MC_T1.glb",
        "./Data/Models/Characters/Savarog/HitReact_Back.glb",
        "./Data/Models/Characters/Savarog/HitReact_Front.glb",
        "./Data/Models/Characters/Savarog/HitReact_Left.glb",
        "./Data/Models/Characters/Savarog/HitReact_Right.glb",
        "./Data/Models/Characters/Savarog/Soul_Siphon.glb",
        "./Data/Models/Characters/Savarog/Soul_Siphon_Targeting.glb",
        "./Data/Models/Characters/Savarog/Stun_End.glb",
        "./Data/Models/Characters/Savarog/Stun_Start.glb",
        "./Data/Models/Characters/Savarog/Stun_Loop.glb",
        "./Data/Models/Characters/Savarog/Swing1_Medium.glb",
        "./Data/Models/Characters/Savarog/Swing1_Return2Idle.glb",
        "./Data/Models/Characters/Savarog/Swing2_Medium.glb",
        "./Data/Models/Characters/Savarog/Swing2_Return2Idle.glb",
        "./Data/Models/Characters/Savarog/Swing3_Medium.glb",
        "./Data/Models/Characters/Savarog/Swing3_Return2Idle.glb",
        "./Data/Models/Characters/Savarog/Death_front.glb",
        "./Data/Models/Characters/Savarog/Victory_Emote.glb",
    };
    skeletalMeshComponent->AppendAnimations(animationFilenames);
    // アニメーションコントローラーを作成
    animationController_ = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    animationController_->AddAnimation("Idle", 0);
    animationController_->AddAnimation("Jog_Fwd", 1);
    animationController_->AddAnimation("Jog_Left", 2);
    animationController_->AddAnimation("Jog_Right", 3);
    animationController_->AddAnimation("LevelStart", 4);
    animationController_->AddAnimation("Recall", 5);
    animationController_->AddAnimation("Emote_Pull_MC_T1", 6);
    animationController_->AddAnimation("HitReact_Back", 7);
    animationController_->AddAnimation("HitReact_Front", 8);
    animationController_->AddAnimation("HitReact_Left", 9);
    animationController_->AddAnimation("HitReact_Right", 10);
    animationController_->AddAnimation("Soul_Siphon", 11);
    animationController_->AddAnimation("Soul_Siphon_Targeting", 12);
    animationController_->AddAnimation("Stun_End", 13);
    animationController_->AddAnimation("Stun_Start", 14);
    animationController_->AddAnimation("Stun_Loop", 15);
    animationController_->AddAnimation("Swing1_Medium", 16);
    animationController_->AddAnimation("Swing1_Return2Idle", 17);
    animationController_->AddAnimation("Swing2_Medium", 18);
    animationController_->AddAnimation("Swing2_Return2Idle", 19);
    animationController_->AddAnimation("Swing3_Medium", 20);
    animationController_->AddAnimation("Swing3_Return2Idle", 21);
    animationController_->AddAnimation("Death_front", 22);
    animationController_->AddAnimation("Victory_Emote", 23);

    // ステートマシンを作成
    stateMachine_ = std::make_shared<StateMachine>();
    stateMachine_->RegisterState(std::make_unique<EnemyIdleState>(this));
    stateMachine_->RegisterState(std::make_unique<EnemyWalkState>(this));
    stateMachine_->RegisterState(std::make_unique<EnemyAttackState>(this));
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");

    // 敵からの攻撃を受ける当たり判定用のコンポーネントを追加
#if 1
    std::shared_ptr<CapsuleComponent> capsuleComponent = this->AddComponent<class CapsuleComponent>("capsuleComponent", "skeletalComponent");
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    height = size.y;
    radius = size.x * 0.25f;
    capsuleComponent->SetRadiusAndHeight(radius, height);
    capsuleComponent->SetMass(40.0f);
    capsuleComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
    capsuleComponent->SetLayer(CollisionLayer::Enemy);
    capsuleComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    capsuleComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::None);
    capsuleComponent->SetCollisionOffsetY(height * 0.5f);
    capsuleComponent->SetIsVisibleDebugBox(false);
    capsuleComponent->Initialize();
#else
    std::shared_ptr<BoxComponent> boxComponent = this->AddComponent<class BoxComponent>("capsuleComponent", "skeletalComponent");
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    boxComponent->SetBoxExtent({ size.x * 0.5f,size.y * 0.5f,size.z * 0.5f });
    boxComponent->SetCollisionOffsetY(size.y * 0.5f);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::Enemy);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

#endif // 0
    // キャラクタームーブメントコンポーネント追加
    characterMovementComponent = AddComponent<CharacterMovementComponent>("characterMovementComponent", "skeletalComponent");
    // 回転コンポーネント追加
    rotationComponent = AddComponent<RotationComponent>("rotationComponent", "skeletalComponent");


    SetPosition(transform.GetLocation());
    SetQuaternionRotation(transform.GetRotation());
    SetScale(transform.GetScale());

}

void BossEnemy::Update(float deltaTime)
{
    DirectX::XMFLOAT3 playerPos = GetOwnerScene()->GetActorManager()->GetActorByName("player")->GetPosition();
    XMVECTOR PlayerPos = XMLoadFloat3(&playerPos);
    DirectX::XMFLOAT3 enemyPos = GetPosition();
    XMVECTOR EnemyPos = XMLoadFloat3(&enemyPos);

    DirectX::XMVECTOR  Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(PlayerPos, EnemyPos));
    XMFLOAT3 direction;
    XMStoreFloat3(&direction, Direction);

    characterMovementComponent->SetMoveDirection(direction);
    rotationComponent->SetDirection(direction);
}
