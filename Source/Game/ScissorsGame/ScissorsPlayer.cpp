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

    // 入力に基づいて移動と回転を更新
    auto intent = inputComponent->GetIntent();
    DirectX::XMFLOAT3 moveDir = { 0,0,0 };

    // 左スティック入力
    float stickX = intent.leftMove.x;
    float stickZ = intent.leftMove.z;

    moveDir.x = stickX;
    moveDir.z = stickZ;

    characterMovementComponent->SetMoveDirection(moveDir);
    rotationComponent->SetDirection(moveDir);


    // ゲームパッドが接続されているか
    bool isGamepad = InputSystem::IsGamepadConnected();


    // 右スティックの強さ
    float stickPower = sqrt(
        intent.rightMove.x * intent.rightMove.x +
        intent.rightMove.y * intent.rightMove.y
    );
    // 0～1にクランプ
    stickPower = std::clamp(stickPower, 0.0f, 1.0f);
    // スティック使ってるか判定
    bool usingStick = (stickPower > 0.2f);

    // スティックRelease検出
    static float prevStickPower = 0.0f;
    bool stickReleased = (prevStickPower > 0.2f && stickPower <= 0.2f);
    prevStickPower = stickPower;


    bool showPreview = false;
    float previewPower = 0.0f;


    if (usingStick)
    {// スティックの最大入力値を更新
        lastStickPower = stickPower;
        showPreview = true;
        previewPower = stickPower;
    }

    Logger::Log("Stick Power: " + std::to_string(stickPower) + (usingStick ? " (Using Stick)" : " (Not Using Stick)"));

    // ボタンRelease
    bool buttonReleased = InputSystem::GetInputState("ScissorsAction", InputStateMask::Release);

    // ハサミ処理
    if (scissorsCount == 2)
    {
        // ゲームパッド
        if (isGamepad)
        {
            // スティック操作
            {
                if (stickReleased)
                {
                    float power = lastStickPower;

                    if (power < 0.2f)
                        power = 0.5f;
                    ThrowScissors(power);
                    // 投げた後、最大入力値をリセット
                    lastStickPower = 0.0f;
                    Logger::Log(U8("スティックで投げる"));
                }
            }
            // ボタンのみ
            //if (!usingStick)
            {
                if (buttonReleased)
                {
                    float power = 0.6f; // 固定距離（調整してOK）

                    ThrowScissors(power);
                    Logger::Log(U8("ボタンで投げる（固定距離）"));
                }
            }
        }
        // マウス（チャージ）
        else
        {
            if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Press))
            {
                isCharging = true;
                chargeTime += deltaTime;

                if (chargeTime > maxChargeTime)
                    chargeTime = maxChargeTime;
                showPreview = true;
                previewPower = chargeTime / maxChargeTime;
            }

            if (buttonReleased)
            {
                if (isCharging)
                {
                    float power = chargeTime / maxChargeTime;

                    ThrowScissors(power);
                    Logger::Log(U8("チャージ投げ"));

                    isCharging = false;
                    chargeTime = 0.0f;

                    showPreview = false;

                }
            }
        }
    }
    else if (scissorsCount == 1)
    {
        if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Trigger))
        {
            PullNearest();
            Logger::Log(U8("ハサミを引き寄せる"));
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

    // デバッグ用：溜めてる間、投げる位置の予測を描画
    if (showPreview)
    {
        float dist = previewPower * maxThrowDistance;

        auto pos = GetPosition();
        auto forward = GetForward();

        DirectX::XMFLOAT3 target =
        {
            pos.x + forward.x * dist,
            pos.y,
            pos.z + forward.z * dist
        };

        DebugRender::DrawSphere(target, 0.3f, { 1,1,0,1 });
    }

    // デバッグ用：ハサミを拾える範囲を描画
    DebugRender::DrawCylinder(
        GetPosition(),
        pickupRange,
        0.1f,
        XMFLOAT4{ 0.0f,1.0f,0.0f,0.5f }
    );
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

// ハサミを投げる
void ScissorsPlayer::ThrowScissors(float power)
{
    if (equippedScissors.size() <= 1) return;

    auto scissors = equippedScissors.back();
    auto s = scissors.lock();
    if (!s) return;

    equippedScissors.pop_back();
    droppedScissors.push_back(scissors);
    scissorsCount--;

    // 方向
    auto dir = GetForward();

    s->Throw(dir, power);
}

void ScissorsPlayer::OnScissorsReturned(ScissorsActor* scissors)
{
    // droppedから探す
    auto it = std::find_if(droppedScissors.begin(), droppedScissors.end(),
        [scissors](const std::weak_ptr<ScissorsActor>& w)
        {
            return w.lock().get() == scissors;
        });

    if (it == droppedScissors.end()) return;

    auto s = it->lock();
    if (!s) return;

    // equippedに戻す
    equippedScissors.push_back(*it);
    droppedScissors.erase(it);

    scissorsCount++;

    // 状態変更
    s->PickUp();
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
    if (minDist < pickupRange)
        return nearest;

    return nullptr;
}