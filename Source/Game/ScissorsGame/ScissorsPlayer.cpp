#include "pch.h"

#include "ScissorsPlayer.h"
#include "Engine/Scene/SceneBase.h"
#include "ScissorsActor.h"

void ScissorsPlayer::Initialize(const Transform& transform)
{
    int maxHp = 10;
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

    switch (state)
    {
    case State::Walking:
        // 移動と回転は通常通り

        // 攻撃入力を検出
        if (InputSystem::GetInputState("ScissorsAttack", InputStateMask::Trigger))
        {// 攻撃
            Attack();
            state = State::Attacking;
        }
        break;
    case State::Attacking:
        // 攻撃中は移動と回転を止める
        moveDir = { 0,0,0 };
        if (!animationController_->IsPlayAnimation())
        {
            state = State::Walking; // アニメーションが終わったら歩行状態に戻す
            PlayAnimation("Walk");
        }
        break;
    }

    characterMovementComponent->SetMoveDirection(moveDir);
    rotationComponent->SetDirection(moveDir);

    {// ステージ外に出ないようにクランプ
        float stageMinX = -0.5f;
        float stageMaxX = 12.5f;
        float stageMinZ = -0.5f;
        float stageMaxZ = 12.5f;

        XMFLOAT3 pos = GetPosition();
        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);
        SetPosition(pos);
    }
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

    // ボタンRelease
    bool buttonReleased = InputSystem::GetInputState("ScissorsAction", InputStateMask::Release);

#if 1
    // Aim作る
    AimData aim = GetAimData(intent, deltaTime);

    // 投げ
    TryAction(aim, stickReleased, buttonReleased);

    // 描画
    DrawPreview(aim);
#else
    if (usingStick)
    {// スティックの最大入力値を更新
        lastStickPower = std::max<float>(stickPower, lastStickPower);
        //lastStickPower = stickPower;
        showPreview = true;
        previewPower = lastStickPower;

#if 0 // 不自然なので一旦無で
        // スティックの方向で回転
        rotationComponent->SetDirection({
        intent.rightMove.x,
        0.0f,
        intent.rightMove.y
            });

#endif // 0 // 不自然なので一旦無で
    }

    // スティック方向 → 3D
    DirectX::XMFLOAT3 throwDir = {
        intent.rightMove.x,
        0.0f,
        intent.rightMove.y
    };
    if (isGamepad)
    {

        // 正規化
        float len = sqrt(throwDir.x * throwDir.x + throwDir.z * throwDir.z);

        static XMFLOAT3 lastThrowDir = { 0,0,1 };

        if (len > 0.001f)
        {
            throwDir.x /= len;
            throwDir.z /= len;

            lastThrowDir = throwDir; // 更新
        }
        else
        {
            throwDir = lastThrowDir; // ← ここ変更
        }
    }


    Logger::Log("Stick Power: " + std::to_string(stickPower) + (usingStick ? " (Using Stick)" : " (Not Using Stick)"));


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
                    ThrowScissors(power, throwDir);
                    // 投げた後、最大入力値をリセット
                    lastStickPower = 0.0f;
                    Logger::Log(U8("スティックで投げる"));
                }
            }
            //// ボタンのみ
            ////if (!usingStick)
            //{
            //    if (buttonReleased)
            //    {
            //        float power = 0.6f; // 固定距離（調整してOK）

            //        ThrowScissors(power);
            //        Logger::Log(U8("ボタンで投げる（固定距離）"));
            //    }
            //}
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
                    XMFLOAT3 dir = GetForward(); // プレイヤーの向きに投げる
                    ThrowScissors(power, dir);
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



    // デバッグ用：溜めてる間、投げる位置の予測を描画
    if (showPreview)
    {
        float dist = previewPower * maxThrowDistance;

        auto pos = GetPosition();


        auto forward = GetForward();

        DirectX::XMFLOAT3 targetPos =
        {
            pos.x + forward.x * dist,
            pos.y,
            pos.z + forward.z * dist
        };
#if 1
        float stageMinX = -0.5f;
        float stageMaxX = 12.5f;
        float stageMinZ = -0.5f;
        float stageMaxZ = 12.5f;

        targetPos.x = std::clamp(targetPos.x, stageMinX, stageMaxX);
        targetPos.z = std::clamp(targetPos.z, stageMinZ, stageMaxZ);

#endif // 0
        DebugRender::DrawSphere(targetPos, 0.3f, { 1,1,0,1 });
    }
#endif // 0

    // デバッグ用：ハサミを拾える範囲を描画
    DebugRender::DrawCylinder(
        GetPosition(),
        pickupRange,
        0.1f,
        XMFLOAT4{ 0.0f,1.0f,0.0f,0.5f }
    );

}

void ScissorsPlayer::TakeDamage(int damage)
{
    hp -= damage;
    Logger::Log("Player took " + std::to_string(damage) + " damage. HP: " + std::to_string(hp));
    if (hp <= 0)
    {
        hp = 0;
        //PlayAnimation("Death", false);
        Logger::Log("Player died.");
        // ここでゲームオーバー処理などを呼び出す
    }
}

// プレイヤーの攻撃処理
void ScissorsPlayer::Attack()
{
    PlayAnimation("Attack", false, true);
    for (auto& w : equippedScissors)
    {
        auto s = w.lock();
        if (!s) continue;

        s->StartAttack();
        Logger::Log(U8("攻撃をする"));
    }

}

// 入力から狙いの情報を取得する
ScissorsPlayer::AimData ScissorsPlayer::GetAimData(const MoveIntent& intent, float deltaTime)
{
    AimData aim{};
    aim.isValid = false;

    bool isGamepad = InputSystem::IsGamepadConnected();

    // ゲームパッド
    if (isGamepad)
    {
        float x = intent.rightMove.x;
        float z = intent.rightMove.y;

        float len = sqrt(x * x + z * z);

        static XMFLOAT3 lastDir = { 0,0,1 };

        if (len > 0.2f)
        {
            aim.dir = { x / len, 0.0f, z / len };
            lastDir = aim.dir;

            lastStickPower = std::max<float>(lastStickPower, len);
        }
        else
        {
            aim.dir = lastDir;
        }

        aim.power = lastStickPower;
        aim.isValid = true;
    }
    // マウス
    else
    {
        if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Press))
        {
            isCharging = true;
            chargeTime += deltaTime;
            chargeTime = std::min<float>(chargeTime, maxChargeTime);
        }

        if (isCharging)
        {
            aim.dir = GetForward();
            aim.power = chargeTime / maxChargeTime;
            aim.isValid = true;
        }
    }

    if (scissorsCount == 2)
        aim.intent = ScissorsIntent::Throw;
    else if (scissorsCount == 1)
    {
        aim.intent = ScissorsIntent::Pull;
        aim.isValid = false; // 引き寄せは方向やパワー関係ないので無効に
    }

    return aim;
}
// 狙いの情報から、ハサミを投げる距離や方向を決定して、投げる処理を試みる
void ScissorsPlayer::TryAction(const AimData& aim, bool stickReleased, bool buttonReleased)
{
    bool isGamepad = InputSystem::IsGamepadConnected();
    if (aim.intent == ScissorsIntent::Throw)
    {
        if (isGamepad)
        {
            if (stickReleased)
            {
                float power = std::max<float>(aim.power, 0.5f);
                ThrowScissors(power, aim.dir);

                lastStickPower = 0.0f;
            }
        }
        else
        {
            if (buttonReleased && isCharging)
            {
                ThrowScissors(aim.power, aim.dir);

                isCharging = false;
                chargeTime = 0.0f;
            }
        }
    }
    else if (aim.intent == ScissorsIntent::Pull)
    {
        if (buttonReleased)
        {
            PullNearest();
        }
    }
}
// 狙いの情報をもとに、投げる前のプレビューを描画する
void ScissorsPlayer::DrawPreview(const AimData& aim)
{
    if (!aim.isValid) return;

    float dist = aim.power * maxThrowDistance;

    auto pos = GetPosition();

    DirectX::XMFLOAT3 target =
    {
        pos.x + aim.dir.x * dist,
        pos.y,
        pos.z + aim.dir.z * dist
    };

    // clamp
    float stageMinX = -0.5f;
    float stageMaxX = 12.5f;
    float stageMinZ = -0.5f;
    float stageMaxZ = 12.5f;

    target.x = std::clamp(target.x, stageMinX, stageMaxX);
    target.z = std::clamp(target.z, stageMinZ, stageMaxZ);

    DebugRender::DrawSphere(target, 0.3f, { 1,1,0,1 });
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
void ScissorsPlayer::ThrowScissors(float power, XMFLOAT3 dir)
{
    if (equippedScissors.size() <= 1) return;

    auto scissors = equippedScissors.back();
    auto s = scissors.lock();
    if (!s) return;

    equippedScissors.pop_back();
    droppedScissors.push_back(scissors);
    scissorsCount--;

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

bool ScissorsPlayer::IsPullFinished()
{
    // 例：ハサミが手元に戻ったら
    return scissorsCount == 2;
}