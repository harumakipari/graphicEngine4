#include "pch.h"

#include "ScissorsPlayer1.h"

#include "ScissorsPlayerStateDerived.h"
#include "YarnEnemyActor.h"
#include "Engine/Scene/SceneBase.h"
#include "Physics/CollisionFunction.h"

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
    controller->AddAnimation("Idle", 0);
    controller->AddAnimation("Death", 1);
    controller->AddAnimation("ChargeDash", 2);
    controller->AddAnimation("Dash", 3);
    controller->AddAnimation("Run", 4);
    controller->AddAnimation("Attack", 5);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    PlayAnimation("Idle");

    // ステートマシンを作成
    stateMachine_ = std::make_shared<StateMachine>();
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerIdleState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerRunningState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerAttackingState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerDashState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerChargeDashState>(this));

    // ステートマシンを character に追加
    this->SetStateMachine(stateMachine_);
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");



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

#if 0
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
#endif // 0

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
                if (stateMachine_->GetStateName() != "Dash")
                    return;

                auto enemy = dynamic_cast<YarnEnemyActor*>(other->GetOwner());
                if (!enemy) return;

                hitStopTimer = 0.1f;
                // ヒット処理
                enemy->OnHitByDash(this);
                // コントローラーを振動させる
                InputSystem::SetVibration(0.6f, 0.08f);
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

    // ダッシュの狙いを表示する矢印のUIコンポーネントを追加
    dashAimArrowComponent = std::make_unique<UIImageComponent>("./Data/Textures/ScissorsUI/Arrow.png", "dashAimArrow");
    auto uiManager = GetOwnerScene()->GetUIManager();
    dashAimArrowComponent->SetWorldPosition({ 0.0f, 0.0f });
    dashAimArrowComponent->SetVisible(true);
    dashAimArrowComponent->SetSize({ 50.0f, 300.0f });
    dashAimArrowComponent->SetPivot({ 0.5f, 1.0f }); // 矢印の根元をプレイヤーの位置に合わせる
    dashAimArrowComponent->SetVisible(false);
    uiManager->Add(dashAimArrowComponent);

    // ダッシュ回数を初期化
    dashCount = maxDashCount;

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

    // 左スティック入力
    float stickX = intent.leftMove.x;
    float stickZ = intent.leftMove.z;

    moveDir.x = stickX;
    moveDir.z = stickZ;

    rotationComponent->SetDirection(GetLookDirection());

    XMFLOAT3 pos = GetPosition();

    {// ステージ外に出ないようにクランプ
        float stageMinX = 1.0f;
        float stageMaxX = 19.5f;
        float stageMinZ = 1.0f;
        float stageMaxZ = 19.5f;

        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);
        SetPosition(pos);
    }

    {// 入力を検知するための処理

        // ゲームパッドが接続されているか
        useGamePad = InputSystem::IsGamepadConnected();

        // 右スティックの強さ
        float stickPower = sqrt(
            intent.rightMove.x * intent.rightMove.x +
            intent.rightMove.y * intent.rightMove.y
        );
        // 0～1にクランプ
        stickPower = std::clamp(stickPower, 0.0f, 1.0f);
        // スティック使ってるか判定
        usingStick = (stickPower > 0.2f);

        // スティックRelease検出
        static float prevStickPower = 0.0f;
        stickReleased = (prevStickPower > 0.2f && stickPower <= 0.2f);
        prevStickPower = stickPower;

        // ボタンRelease
        bool buttonReleased = InputSystem::GetInputState("ScissorsAction", InputStateMask::Release);

        //  これでダッシュの方向や溜めの強さを決める
        aimData = GetAimData(intent, deltaTime);

        if (useGamePad)
        {// ゲームパッド使用
            // スティック離したとき
            triggerDash = stickReleased;
            // ダッシュ溜めトリガー
            if (usingStick && !preUsingStick)
            {
                triggerChargeDash = true;
            }
            else
            {
                triggerChargeDash = false;
            }
            //triggerChargeDash = usingStick;

            preUsingStick = usingStick;

        }
        else
        {//　ゲームパッド使用してない
            // ボタン離したとき（左マウス)
            triggerDash = buttonReleased;
            // ダッシュ溜めトリガー
            triggerChargeDash = InputSystem::GetInputState("ScissorsAction", InputStateMask::Trigger);
        }

        attackTrigger = InputSystem::GetInputState("ScissorsAttack", InputStateMask::Trigger);

    }

    // ダッシュ回復
    RecoverDash(deltaTime);


    // ダッシュの狙いを表示する矢印のUIを更新
    XMFLOAT2 uiPos = WorldToUI(pos);
    if (dashAimArrowComponent)
    {
        dashAimArrowComponent->SetWorldPosition({ uiPos.x, uiPos.y });
        float angle = DirectX::XMConvertToDegrees(atan2f(aimData.dir.x, aimData.dir.z));
        dashAimArrowComponent->SetWorldAngleDegree(angle);
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
            //aim.dir = moveDir; //　マウスでは移動方向にダッシュする
            aim.dir = GetForward();
            aim.power = chargeTime / maxChargeTime;
            aim.power = 1.0f;

            aim.isValid = true;
        }
    }

    return aim;
}

// どの方向を向くか
DirectX::XMFLOAT3 ScissorsPlayer1::GetLookDirection() const
{
    if (useGamePad && stateMachine_->GetStateName() == "ChargeDash")
    {
        return aimData.dir;
    }
    else
    {
        return moveDir;
    }
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
void ScissorsPlayer1::DoAttackHit()
{
    auto enemies = GetOwnerScene()->GetActorManager()->GetActorsOfType<YarnEnemyActor>();
    Logger::Log(U8("攻撃をする"));

    for (auto& enemy : enemies)
    {
        if (!enemy) continue;

        auto p = GetPosition();
        auto e = enemy->GetPosition();

        // 敵へのベクトル
        float dx = e.x - p.x;
        float dz = e.z - p.z;

        float distSq = dx * dx + dz * dz;
        float attackRange = 2.5f;

        DebugRender::DrawSphere(p, attackRange, { 1,1,1,1 });

        if (distSq > attackRange * attackRange)
            continue;

        // 正規化
        float len = sqrtf(dx * dx + dz * dz);
        dx /= len;
        dz /= len;

        // プレイヤーの前方向（Z+方向）
        DirectX::XMFLOAT3 forward = GetForward();

        float dot = dx * forward.x + dz * forward.z;

        float angleCos = cosf(DirectX::XMConvertToRadians(60.0f));

        if (dot > angleCos)
        {
            enemy->TakeDamage(1);
            Logger::Log(U8("敵にヒット！"));
        }
    }
}


// ダッシュの回数を回復する関数　
void ScissorsPlayer1::RecoverDash(float deltaTime)
{
    // ダッシュ回復
    dashRecoverTimer += deltaTime;

    if (dashRecoverTimer >= dashRecoverInterval)
    {
        dashRecoverTimer = 0.0f;

        if (dashCount < maxDashCount)
        {
            dashCount++;
            Logger::Log("Dash recovered. Current dash count: " + std::to_string(dashCount));
        }
    }
}

//　ダッシュを使用する関数　これを呼ぶとダッシュの残り回数が減る
void ScissorsPlayer1::UseDash()
{
    if (dashCount > 0)
    {
        dashCount--;
    }
    Logger::Log("Dash used. Remaining dash count: " + std::to_string(dashCount));
}

// ダッシュが失敗した時に呼ぶ関数　これを呼ぶとダッシュの残り回数が減らない
void ScissorsPlayer1::FailDash()
{
    // ダッシュ失敗の処理
    Logger::Log("Dash failed. Remaining dash count: " + std::to_string(dashCount));
}

// ダッシュを止める処理　これを呼ぶとダッシュが止まる
void ScissorsPlayer1::StopDash()
{
    // ダッシュ停止の処理
    stateMachine_->ChangeState("Idle");
    Logger::Log("Dash stopped.");
}