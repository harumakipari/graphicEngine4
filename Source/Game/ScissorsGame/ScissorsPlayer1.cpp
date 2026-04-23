#include "pch.h"

#include "ScissorsPlayer1.h"

#include "NeedleEnemyActor.h"
#include "RibbonWallActor.h"
#include "ScissorsPlayerStateDerived.h"
#include "YarnEnemyActor.h"
#include "ScissorsGameEnemyBaseActor.h"
#include "Engine/Scene/SceneBase.h"
#include "Engine/Utility/Time.h"
#include "Physics/CollisionFunction.h"

void ScissorsPlayer1::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    //skeletalMeshComponent->SetModel("./Data/TeamModels/Player/player.gltf", false, true);

    skeletalMeshComponent->SetModel("./Data/TeamModels/Player/ScissorsPlayer.glb", false, true);

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
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerStunState>(this));

    // ステートマシンを character に追加
    this->SetStateMachine(stateMachine_);
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");

    // 当たり判定
    {
        sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        radius = playerRadius;
        height = 0.8f;
        mass = 60.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetRelativeLocationDirect({ 0.0f,height,0.0f });
        sphereComponent->SetMass(mass);
        sphereComponent->SetLayer(CollisionLayer::Player);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::RibbonWall, CollisionComponent::CollisionResponse::Block);
        sphereComponent->Initialize();
        sphereComponent->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (stateMachine_->GetStateName() == "Attack") // 攻撃中の時はダメージを受けない
                    return;

                if (damageCooldownTimer > 0.0f) return;


                if (other->GetCollisionLayer() == CollisionHelper::ToBit(CollisionLayer::Enemy))
                {
                    if (auto enemy = dynamic_cast <ScissorsGameEnemyBase*>(other->GetOwner()))
                    {
                        if (enemy->IsDead())
                        {// 敵が死んでいたら
                            return;
                        }
                    }
                    debugPlayerCollisionColor = { 1.0f,0.0f,0.0f,1.0f };
                    TakeDamage(1);
                    damageCooldownTimer = damageCooldownInterval; // 無敵時間を設定

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

    // ダッシュ攻撃用の当たり判定
    {
        dashAttackSphere = this->AddComponent<SphereComponent>("dashAttackSphere", parentName);
        dashAttackSphere->SetRelativeLocationDirect({ 0.0f,height,0.7f });
        dashAttackSphere->SetRadius(dashAttackRange);
        dashAttackSphere->SetLayer(CollisionLayer::PlayerWeapon);
        dashAttackSphere->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
        dashAttackSphere->SetCollisionOffsetY(height);
        dashAttackSphere->Initialize();
        dashAttackSphere->SetActive(false); // ←通常はOFF
        dashAttackSphere->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (stateMachine_->GetStateName() != "Dash")
                    return;

                auto enemy = dynamic_cast<ScissorsGameEnemyBase*>(other->GetOwner());
                if (!enemy) return;

                if (enemy->IsDead())
                {// 敵が死亡して場合
                    return;
                }

                CoreAudio::PlayOneShot(L"./Data/Sound/SE1/enemyHit_strong.wav", 1.0f);
                //CoreAudio::PlayOneShot(L"./Data/Sound/SE1/scissors_attack.wav",1.0f);

                // ヒット処理と倒したかどうかを取得する
                bool isKilled = enemy->OnHitByDash(this, dashDamage);

                // スコアデータを取得する
                auto data = enemy->GetScoreData();

                // スコア処理　足されたスコアを取得する
                int addScore = scoreSystem.ProcessHit(data, isKilled);

                // スロー再生
                //Time::SetSlow(0.5f, 0.3f);

                // ヒットストップ
                hitStopTimer = hitStopDuration;
            }
        );
    }

    // ハサミ攻撃用の当たり判定
    {
        scissorsAttackSphere = this->AddComponent<SphereComponent>("scissorsAttackSphere", parentName);
        scissorsAttackSphere->SetRelativeLocationDirect({ 0.0f,height,1.1f });
        scissorsAttackSphere->SetRadius(scissorsAttackRange);
        scissorsAttackSphere->SetLayer(CollisionLayer::PlayerWeapon);
        scissorsAttackSphere->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
        scissorsAttackSphere->SetResponseToLayer(CollisionLayer::RibbonWall, CollisionComponent::CollisionResponse::Trigger);
        scissorsAttackSphere->Initialize();
        scissorsAttackSphere->SetActive(false); // ←通常はOFF
        scissorsAttackSphere->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (stateMachine_->GetStateName() != "Attack")
                    return;

#if 0
                if (hasDamageEnemy) // 既にダメージが入っていたら無視
                    return;

#endif // 0

                auto wall = dynamic_cast<RibbonWallActor*>(other->GetOwner());
                if (wall)
                {// 壁が切られたら
                    auto needleEnemy = wall->ownerEnemy.lock();
                    needleEnemy->BreakAllWalls(); // 壁をすべて破壊する
                    return;
                }

                auto enemy = dynamic_cast<ScissorsGameEnemyBase*>(other->GetOwner());
                if (!enemy) return;

                if (hitEnemies.contains(enemy)) return;

                hitEnemies.insert(enemy);

                // ヒット処理と倒したかどうかを取得する
                bool isKilled = enemy->OnHitByAttack(this, scissorsDamage);
                Logger::Log(U8("敵にヒット！"));

                hasDamageEnemy = true;

                // スコアデータを取得する
                auto data = enemy->GetScoreData();

                // スコア処理　足されたスコアを取得する
                int addScore = scoreSystem.ProcessHit(data, isKilled);

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
    dashAimArrowComponent->SetSize({ 300.0f, 50.0f });
    dashAimArrowComponent->SetPivot({ 0.0f, 0.5f }); // 矢印の根元をプレイヤーの位置に合わせる
    dashAimArrowComponent->SetVisible(false);
    uiManager->Add(dashAimArrowComponent);

    // ダッシュ回数を初期化
    dashCount = maxDashCount;

    // 歩行音のオーディオコンポーネント
    footstepAudioComponent = AddComponent<CoreAudioSourceComponent>("footstepAudioComponent", parentName);
    footstepAudioComponent->SetSource(L"./Data/Sound/SE1/playerRun.wav");
    footstepAudioComponent->SetVolume(0.2f);
    footstepAudioComponent->SetLoop(true);

    // チャージ音のオーディオコンポーネント
    chargeAudioComponent = AddComponent<CoreAudioSourceComponent>("chargeAudioComponent", parentName);
    chargeAudioComponent->SetSource(L"./Data/Sound/SE1/charge2.wav");
    chargeAudioComponent->SetVolume(0.5f);
    chargeAudioComponent->SetLoop(true);

    // プレイヤーのHPを表示するUIを作成
    {
        int maxHp = 5;
        hp = maxHp;
        for (int i = 0; i < maxHp; i++)
        {
            auto hpUI = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/heart.png", "hpUI");

            // 横に並べる
            hpUI->SetWorldPosition({ 100.0f + i * 160.0f, 50.0f });

            hpUI->SetSize({ 150.0f, 150.0f });
            hpUI->SetVisible(true);

            uiManager->Add(hpUI);
            hpUiComponents.push_back(hpUI);
        }
    }

    // 軌跡初期化
    trail.Initialize();

}

void ScissorsPlayer1::Update(float deltaTime)
{
    auto scene = GetOwnerScene();
    if (scene->IsPaused())
    {// ポーズ中は入力を受け付けない 歩行音も止める
        return;
    }

    Character::Update(deltaTime);

    //　軌跡更新
    trail.UpdateTrail(deltaTime);

    if (damageCooldownTimer > 0.0f)
    {// ダメージクールダウン中は無敵
        damageCooldownTimer -= deltaTime;
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


        if (!InputSystem::isUIUsingMouse)
        {
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
        else
        {
            // UI操作中は入力無効
            triggerDash = false;
            triggerChargeDash = false;
            attackTrigger = false;
        }

    }

    // ダッシュ回復
    RecoverDash(deltaTime);

    // スコアシステムの更新
    scoreSystem.Update(deltaTime);

    // ダッシュの狙いを表示する矢印のUIを更新
    XMFLOAT2 uiPos = WorldToUI(pos);
    if (dashAimArrowComponent)
    {
        dashAimArrowComponent->SetWorldPosition({ uiPos.x, uiPos.y });
    }

    // playerの当たり判定をデバッグ表示
    DebugRender::DrawSphere(sphereComponent->GetComponentLocation(), playerRadius, debugPlayerCollisionColor, 0, true);
    // ダッシュ攻撃の当たり判定をデバッグ表示
    DebugRender::DrawSphere(dashAttackSphere->GetComponentLocation(), dashAttackRange, debugDashCollisionColor, 0, true);
    // ハサミ攻撃の当たり判定をデバッグ表示
    DebugRender::DrawSphere(scissorsAttackSphere->GetComponentLocation(), scissorsAttackRange, debugScissorsCollisionColor, 0, true);

    debugPlayerCollisionColor = { 1,1,1,1 };

}

void ScissorsPlayer1::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    Character::DrawImGuiDetails();
    ImGui::DragFloat(U8("プレイヤーの当たり判定"), &playerRadius);
    ImGui::DragFloat(U8("ダッシュの当たり判定"), &dashAttackRange);
    ImGui::SliderInt(U8("ハサミ攻撃時のダメージ量"), &scissorsDamage, 1, 10);
    ImGui::SliderInt(U8("ダッシュ攻撃時のダメージ量"), &dashDamage, 1, 10);
    ImGui::Text("HP: %d", hp);
    ImGui::Text("Dash Count: %d", dashCount);
    ImGui::Text("Current State: %s", stateMachine_->GetStateName());
#endif
}

// 軌跡を描画する処理
void ScissorsPlayer1::RenderTrail(ID3D11DeviceContext* immediateContext)
{
    trail.Render(immediateContext);
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

            // チャージ開始と継続
            isCharging = true;
            chargeTime += deltaTime;
            chargeTime = std::min<float>(chargeTime, maxChargeTime);

            lastStickPower = std::max<float>(lastStickPower, len);
        }
        else
        {
            aim.dir = lastDir;
        }

        aim.power = chargeTime / maxChargeTime;
        //aim.power = lastStickPower;
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
            //aim.power = 1.0f;
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

// ダメージを受けたときの処理
void ScissorsPlayer1::TakeDamage(int damage)
{
    // デバック用に当たり判定を赤くする
    debugPlayerCollisionColor = { 1,0,0,1 };

    hp -= damage;
    Logger::Log("Player took " + std::to_string(damage) + " damage. HP: " + std::to_string(hp));
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/playerDamage.wav");
    if (hp <= 0)
    {
        hp = 0;
        //PlayAnimation("Death", false);
        Logger::Log("Player died.");
        // ここでゲームオーバー処理などを呼び出す
    }
    // HPを表示するUIを更新
    UpdateHpUI();

    // ダメージを受けたらコンボをリセットする
    scoreSystem.ResetCombo();
}

// プレイヤーの攻撃処理
void ScissorsPlayer1::DoAttackHit()
{
    // 攻撃が当たった敵を記録するためのセットをクリア
    hitEnemies.clear();

    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/scissors_attack.wav", 2.0f);

    auto enemies = GetOwnerScene()->GetActorManager()->GetActorsOfType<ScissorsGameEnemyBase>();
    Logger::Log(U8("攻撃をする"));

#if 0
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
#endif // 0

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

// HPを表示するUIを更新する関数　
void ScissorsPlayer1::UpdateHpUI()
{
    for (int i = 0; i < hpUiComponents.size(); i++)
    {
        if (i < hp)
        {
            hpUiComponents[i]->SetVisible(true);
        }
        else
        {
            hpUiComponents[i]->SetVisible(false);
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
    chargeTime = 0.0f;

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

// ポーズの時に呼ぶ関数　これを呼ぶと歩きのSEが止まる
void ScissorsPlayer1::OnPause()
{
    if (auto state = GetStateMachine()->GetCurrentState())
    {
        // 歩行のSEを止める
        if (footstepAudioComponent)
        {
            footstepAudioComponent->Stop();
        }

        state->Exit(); // 現在のステートから抜ける
        stateMachine_->ChangeState("Idle"); // ポーズ中はIdleステートにする チャージダッシュ時もこれで止める

    }
}

// ダッシュの方向転換をする関数
void ScissorsPlayer1::RedirectDash(const DirectX::XMFLOAT3& newDir)
{
    // 正規化
    DirectX::XMFLOAT3 dir = newDir;
    float len = sqrt(dir.x * dir.x + dir.z * dir.z);

    if (len > 0.0001f)
    {
        dir.x /= len;
        dir.z /= len;
    }

    // 向き更新
    moveDir = dir;

    // ダッシュステート側にも反映（重要）
    if (auto state = stateMachine_->GetCurrentState())
    {
        if (stateMachine_->GetStateName() == "Dash")
        {
            // DashStateに方向を持たせてるならそこにも渡す
            auto dashState = static_cast<ScissorsPlayerDashState*>(state);
            dashState->Redirect(dir);
        }
    }

    // 回転も更新
    rotationComponent->SetDirection(dir);
}

// 星を生成する
void ScissorsPlayer1::SpawnStarParticle(DirectX::XMFLOAT3 pos, XMFLOAT3 playerForward)
{
    // 星のテクスチャを生成
    DirectX::XMFLOAT2 uiPos = WorldToUI(pos);
    //auto starTex = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/star.png","star");
    //auto starTex = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png",uiPos);
    auto starTex = std::make_shared<UIDashEffect>("./Data/Textures/ScissorsUI/star.png", uiPos, DirectX::XMFLOAT2{ playerForward.x,playerForward.z });
    auto uiManager = GetOwnerScene()->GetUIManager();
    starTex->SetWorldPosition(uiPos);
    starTex->SetVisible(true);
    starTex->SetSize({ 50.0f, 50.0f });
    starTex->SetPivot({ 0.5f, 0.5f });
    uiManager->Add(starTex);
}