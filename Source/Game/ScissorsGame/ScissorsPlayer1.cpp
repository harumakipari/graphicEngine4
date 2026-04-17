#include "pch.h"

#include "ScissorsPlayer1.h"

#include "YarnEnemyActor.h"
#include "Engine/Scene/SceneBase.h"

void ScissorsPlayer1::Initialize(const Transform& transform)
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

    // 当たり判定
    {
        std::shared_ptr<SphereComponent> sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = { 0.5f,0.5f,0.5f };
        radius = size.x;
        height = size.y;
        mass = 60.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(mass);
        sphereComponent->SetLayer(CollisionLayer::Player);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetCollisionOffsetY(height * 0.5f);
        sphereComponent->Initialize();
        sphereComponent->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (damageCooldown > 0.0f) return;

                if (other->GetCollisionLayer() == CollisionHelper::ToBit(CollisionLayer::Enemy))
                {
                    TakeDamage(1);
                    damageCooldown = 0.5f; // 0.5秒無敵

                    // ノックバック
                    DirectX::XMFLOAT3 dir =
                    {
                        self->GetOwner()->GetPosition().x - other->GetOwner()->GetPosition().x,
                        0,
                        self->GetOwner()->GetPosition().z - other->GetOwner()->GetPosition().z
                    };

                    float len = sqrt(dir.x * dir.x + dir.z * dir.z);
                    if (len > 0.0001f)
                    {
                        dir.x /= len;
                        dir.z /= len;
                    }
                    XMFLOAT3 impulse = { dir.x * 10.0f, 0.0f, dir.z * 10.0f }; // ノックバックの強さ
                    if (characterMovementComponent)
                    {
                        characterMovementComponent->AddImpulse(impulse);
                    }

                    // ダメージを受けたときのエフェクトや音をここで再生する
                }
            }
        );
    }

    {
        auto attackSphere = this->AddComponent<SphereComponent>("attackSphere", parentName);

        attackSphere->SetRadius(0.8f);
        attackSphere->SetLayer(CollisionLayer::PlayerWeapon);

        attackSphere->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);

        attackSphere->SetCollisionOffsetY(height * 0.5f);

        attackSphere->Initialize();

        attackSphere->SetActive(false); // ←通常はOFF

        attackSphere->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (state != State::Dashing) return;

                auto enemy = dynamic_cast<YarnEnemyActor*>(other->GetOwner());
                if (!enemy) return;

                // ★ヒット処理
                enemy->TakeDamage(2);

                //// ★ノックバック
                //XMFLOAT3 dir =
                //{
                //    other->GetOwner()->GetPosition().x - GetPosition().x,
                //    0,
                //    other->GetOwner()->GetPosition().z - GetPosition().z
                //};

                //float len = sqrt(dir.x * dir.x + dir.z * dir.z);
                //if (len > 0.001f)
                //{
                //    dir.x /= len;
                //    dir.z /= len;
                //}

                //// 敵側に吹っ飛ばし（敵にMovementあれば）
                //if (auto movement = other->GetOwner()->GetComponent<CharacterMovementComponent>())
                //{
                //    movement->AddImpulse({ dir.x * 8.0f, 0, dir.z * 8.0f });
                //}

                // ★ヒットしたら止める（好み）
                //state = State::Walking;
            }
        );
    }

    // 入力用のコンポーネントを追加
    inputComponent = this->AddComponent<class InputComponent>("inputComponent", parentName);

    // 移動用コンポーネントを追加
    characterMovementComponent = this->AddComponent<CharacterMovementComponent>("movementComponent", parentName);
    characterMovementComponent->SetUseGravity(false);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);
    rotationComponent->SetRotateTime(0.1f); // 回転を速くする
}

void ScissorsPlayer1::Update(float deltaTime)
{

    Character::Update(deltaTime);

    if (damageCooldown > 0.0f)
    {// ダメージクールダウン中は無敵
        damageCooldown -= deltaTime;
    }

    // 入力に基づいて移動と回転を更新
    auto intent = inputComponent->GetIntent();
    DirectX::XMFLOAT3 moveDir = { 0,0,0 };

    // 左スティック入力
    float stickX = intent.leftMove.x;
    float stickZ = intent.leftMove.z;

    moveDir.x = stickX;
    moveDir.z = stickZ;

    //characterMovementComponent->SetMoveDirection(moveDir);
    //rotationComponent->SetDirection(moveDir);

    {// ステージ外に出ないようにクランプ
        float stageMinX = 1.0f;
        float stageMaxX = 19.5f;
        float stageMinZ = 1.0f;
        float stageMaxZ = 19.5f;

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

    //  ここを修正
    auto aim = GetAimData(intent, deltaTime);

    bool triggerDash = false;
    if (isGamepad)
    {
        triggerDash = stickReleased;
    }
    else
    {
        triggerDash = buttonReleased;
    }

    bool hasMoveInput = (fabs(moveDir.x) > 0.1f || fabs(moveDir.z) > 0.1f);
    bool attackTrigger = InputSystem::GetInputState("ScissorsAttack", InputStateMask::Trigger);
    switch (state)
    {
    case State::Idle:
        //if (hasMoveInput)
        //{
        //    state = State::Walking;
        //    PlayAnimation("Walk", true, true);
        //}
        if (attackTrigger)
        {
            // 攻撃開始処理
            Attack();
            break;
        }
        break;
    case State::Attacking:
        // 移動止める
        characterMovementComponent->SetMoveDirection({ 0,0,0 });

        // アニメーション終わったら戻る
        if (!animationController_->IsPlayAnimation())
        {
            state = State::Walking;
            PlayAnimation("Walk", true, true);
        }
        break;
    case State::Walking:
        //if (!hasMoveInput)
        //{
        //    state = State::Idle;
        //    PlayAnimation("Death", true, true);
        //    break;
        //}
        if (attackTrigger)
        {
            // 攻撃開始処理
            Attack();
            break;
        }
        if (state != State::Dashing && state != State::ChargingDash)
        {
            float speed = 5.0f;
            characterMovementComponent->SetSpeed(speed);
            characterMovementComponent->SetMoveDirection(moveDir);
            rotationComponent->SetDirection(moveDir);
        }
        if (isGamepad)
        {
            if (usingStick && dashCount > 0)
            {
                dashChargeTime = 0.0f;
                state = State::ChargingDash;
            }
        }
        else
        {
            if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Press) && dashCount > 0)
            {
                dashChargeTime = 0.0f;
                state = State::ChargingDash;
            }
        }
        break;
    case State::ChargingDash:
    {
        // 移動止める
        characterMovementComponent->SetMoveDirection({ 0,0,0 });

        dashChargeTime += deltaTime;
        dashChargeTime = std::min<float>(dashChargeTime, maxDashChargeTime);

        if (triggerDash)
        {
            XMFLOAT3 dir = aim.dir;

            float len = sqrt(dir.x * dir.x + dir.z * dir.z);

            if (len > 0.1f)
            {
                dashDir = { dir.x / len, 0.0f, dir.z / len };

                float power = aim.power;

                float dashDistance =
                    minDashDistance +
                    (maxDashDistance - minDashDistance) * power;

                dashSpeed = dashDistance / dashTime;

                dashTimer = 0.0f;

                dashCount--;

                state = State::Dashing;

                PlayAnimation("Attack", false, true);

                // ★マウス用リセット
                isCharging = false;
                chargeTime = 0.0f;
            }
            else
            {
                state = State::Walking;
                PlayAnimation("Walk", true, true);
            }
        }
    }
    break;
    case State::Dashing:

    {
        dashTimer += deltaTime;

        // 突進
        DirectX::XMFLOAT3 velocity =
        {
            dashDir.x * dashSpeed,
            0.0f,
            dashDir.z * dashSpeed
        };

        //characterMovementComponent->AddImpulse(velocity);
        characterMovementComponent->SetMoveDirection(dashDir);
        characterMovementComponent->SetSpeed(dashSpeed);
        // 向き固定
        //rotationComponent->SetDirection(dashDir);

        // 終了
        if (dashTimer >= dashTime)
        {
            state = State::Walking;
            PlayAnimation("Walk", true, true);
        }
    }
    break;
    }

    // デバッグ用：ハサミを拾える範囲を描画
    DebugRender::DrawCylinder(
        GetPosition(),
        pickupRange,
        0.1f,
        XMFLOAT4{ 0.0f,1.0f,0.0f,0.5f }
    );

    // ダッシュ回復
    dashRecoverTimer += deltaTime;

    if (dashRecoverTimer >= dashRecoverInterval)
    {
        dashRecoverTimer = 0.0f;

        if (dashCount < maxDashCount)
        {
            dashCount++;
        }
    }

}

// 入力から狙いの情報を取得する
ScissorsPlayer1::AimData ScissorsPlayer1::GetAimData(const MoveIntent& intent, float deltaTime)
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
            aim.power = 1.0f;

            aim.isValid = true;
        }
    }

    return aim;
}

void ScissorsPlayer1::TakeDamage(int damage)
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
void ScissorsPlayer1::Attack()
{
    PlayAnimation("Attack", false, true);
    state = State::Attacking;

    Logger::Log(U8("攻撃をする"));

    // プレイヤーの前方向
    XMFLOAT3 forward = GetForward();

    // 攻撃中心位置（前に1m）
    XMFLOAT3 center =
    {
        GetPosition().x + forward.x * 1.0f,
        GetPosition().y,
        GetPosition().z + forward.z * 1.0f
    };

    float attackRadius = 3.0f;

    for (auto& actor : GetOwnerScene()->GetActorManager()->GetActorsOfType<YarnEnemyActor>())
    {
        XMFLOAT3 enemyPos = actor->GetPosition();

        // 距離チェック
        float dx = enemyPos.x - center.x;
        float dz = enemyPos.z - center.z;
        float distSq = dx * dx + dz * dz;

        if (distSq > attackRadius * attackRadius)
            continue;

        // 前方向チェック（任意だけど精度UP）
        XMFLOAT3 toEnemy =
        {
            enemyPos.x - GetPosition().x,
            0.0f,
            enemyPos.z - GetPosition().z
        };

        float len = sqrt(toEnemy.x * toEnemy.x + toEnemy.z * toEnemy.z);
        if (len > 0.001f)
        {
            toEnemy.x /= len;
            toEnemy.z /= len;
        }

        // ドット積（前にいるか）
        float dot = forward.x * toEnemy.x + forward.z * toEnemy.z;

        if (dot < 0.3f) // ←角度制限（調整ポイント）
            continue;

        // ヒット！
        actor->TakeDamage(2);
        Logger::Log(U8("敵にヒット！"));
    }
}


