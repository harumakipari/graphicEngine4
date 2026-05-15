#include "pch.h"

#include "ScissorsPlayer1.h"

#include <algorithm>

#include "BonusUiActor.h"
#include "ButtonCoinActor.h"
#include "EnemyBase.h"
#include "NeedleEnemyActor.h"
#include "RabbitBossEnemy.h"
#include "RibbonWallActor.h"
#include "ScissorsPlayerStateDerived.h"
#include "YarnEnemyActor.h"
#include "ScissorsGameEnemyBaseActor.h"
#include "ScissorsGameManager.h"
#include "ScissorsGameState.h"
#include "ScorePopupActor.h"
#include "TutorialActor.h"
#include "Engine/Scene/SceneBase.h"
#include "Engine/Utility/Time.h"
#include "Physics/CollisionFunction.h"

void ScissorsPlayer1::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Player/ScissorsPlayer.gltf", false, true);
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGamePlayerPS";
    //skeletalMeshComponent->overrideDeferredPipelineName = "GameModelColorFilterPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Player;

    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    //controller->AddAnimation("Idle", 0);
    //controller->AddAnimation("ChargeDash", 1);
    //controller->AddAnimation("Dash", 2);
    //controller->AddAnimation("Run", 3);

    controller->AddAnimation("Idle", 0);
    controller->AddAnimation("Run", 1);     // 走り
    controller->AddAnimation("Dash", 2);    // ダッシュ中
    controller->AddAnimation("ChargeDash", 3);  // チャージ中
    controller->AddAnimation("PreDown", 4);    // 倒れる前
    controller->AddAnimation("Down", 5);    // 倒れる


    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    PlayAnimation("Idle");

    // ステートマシンを作成
    stateMachine_ = std::make_shared<StateMachine>();
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerIdleState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerRunningState>(this));
    //stateMachine_->RegisterState(std::make_unique<ScissorsPlayerAttackingState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerDashState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerChargeDashState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerStunState>(this));
    stateMachine_->RegisterState(std::make_unique<ScissorsPlayerDeathState>(this));

    // ステートマシンを character に追加
    this->SetStateMachine(stateMachine_);
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");

    // 当たり判定
    {
        sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        radius = playerRadius;
        height = 0.2f;
        mass = 60.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetRelativeLocationDirect({ 0.0f,height,0.0f });
        sphereComponent->SetMass(mass);
        sphereComponent->SetLayer(CollisionLayer::Player);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::Boss, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::Bobbin, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::EnemyRedirect, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->Initialize();
        sphereComponent->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (stateMachine_->GetStateName() == "Attack") // 攻撃中の時はダメージを受けない
                    return;

                if (stateMachine_->GetStateName() == "Dash") // ダッシュ中の時はダメージを受けない
                    return;

                if (damageCooldownTimer > 0.0f)
                {// ダメージ後のクールタイムを設定する
                    return;
                }

                if (knockBackTimer > 0.0f)
                {// ノックバック後のクールタイムを設定する
                    return;
                }

                if (postDashInvincibleTimer > 0.0f)
                {// ダッシュ後のクールタイムを設定する
                    return;
                }

                uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Enemy) | CollisionHelper::ToBit(CollisionLayer::Boss);
                if (other->GetCollisionLayer() & mask)
                {
                    if (auto enemy = dynamic_cast <EnemyBase*>(other->GetOwner()))
                    {
                        if (enemy->IsDead())
                        {// 敵が死んでいたら
                            return;
                        }
                        if (enemy->GetState() == EnemyBase::YarnState::Tied)
                        {// 敵が玉止めされていたら
                            return;
                        }
                    }
                    debugPlayerCollisionColor = { 1.0f,0.0f,0.0f,1.0f };
                    TakeDamage(2);
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
        dashAttackBox = this->AddComponent<BoxComponent>("dashAttackBox", parentName);
        dashAttackBox->SetRelativeLocationDirect({ 0.0f,height,1.1f });
        dashAttackBox->SetHalfBoxExtent({ dashAttackRange,height * 0.5f,dashAttackRange });
        dashAttackBox->SetLayer(CollisionLayer::PlayerWeapon);
        dashAttackBox->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
        dashAttackBox->SetResponseToLayer(CollisionLayer::Bobbin, CollisionComponent::CollisionResponse::Trigger);
        dashAttackBox->SetCollisionOffsetY(height);
        dashAttackBox->Initialize();
        dashAttackBox->SetActive(false); // ←通常はOFF
        dashAttackBox->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                if (stateMachine_->GetStateName() != "Dash")
                    return;


                auto enemy = dynamic_cast<EnemyBase*>(other->GetOwner());
                if (!enemy) return;

                if (enemy->IsDead())
                {// 敵が死亡している場合
                    return;
                }

                // 同じセグメントなら無視
                if (enemy->lastHitSegment == currentSegment)
                {
                    return;
                }

                enemy->lastHitSegment = currentSegment;



                if (auto boss = dynamic_cast<RabbitBossEnemyActor*>(enemy))
                {
                    BossDamageContext bossDamageContext = {};
                    bossDamageContext.killedEnemyBeforeHitCount = killedEnemyCountInDash;
                    bossDamageContext.baseDamage = 12.0f;
                    bossDamageContext.isBossStunned = boss->IsStunned();
                    bossDamageContext.suppressBombSpawn = hitBobbinInThisDash;

                    float damage = boss->ComputeDamage(bossDamageContext);
                    Logger::Log(U8("ボスにダメージ：") + std::to_string(damage));
                    boss->TakeDamage(static_cast<int>(damage));

#if 1
                    // 無敵時間を追加 ノックバックのための
                    knockBackTimer = knockBackInterval; // 無敵時間を設定

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
                    XMFLOAT3 impulse = { dir.x * 15.0f, 0.0f, dir.z * 15.0f }; // ノックバックの強さ
                    if (characterMovementComponent)
                    {
                        characterMovementComponent->AddImpulse(impulse);
                    }
#endif // 0
                }
                else
                {
                    //CoreAudio::PlayOneShot(L"./Data/Sound/SE1/boss_hit_se.wav");

                    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/enemyHit_strong.wav", 1.0f);

                    bool isReflected = (currentSegment > 0);    // 反射かどうか


                    // ヒット処理と倒したかどうかを取得する
                    bool isKilled = enemy->OnHitByDash(isReflected);

                    if (isKilled)
                    {
                        if (isReflected)
                        {// 反射キル
                            DashHitInfo info;
                            info.enemy = enemy;
                            info.isReflected = true;
                            dashHits.push_back(info);

                        }
                        else
                        {
#if 0
                            // スコアデータを取得する
                            auto data = enemy->GetScoreData();
                            auto pos = enemy->GetPosition();
                            // スコア処理　足されたスコアを取得する コンボ加算
                            int addScore = ScoreSystem::ProcessHit(data, isKilled);
                            SpawnScorePopup(pos, addScore);
#endif // 0
                        }
                        // スロー再生
                        //Time::SetSlow(0.5f, 0.3f);

                        // ヒットストップ
                        hitStopTimer = hitStopDuration;
                        killedEnemyCountInDash++; // ダッシュ中に倒した敵をカウントする
                    }
                }
            }
        );
    }

    // 入力用のコンポーネントを追加
    inputComponent = this->AddComponent<class InputComponent>("inputComponent", parentName);

    // 移動用コンポーネントを追加
    characterMovementComponent = this->AddComponent<CharacterMovementComponent>("movementComponent", parentName);
    characterMovementComponent->SetUseGravity(false);
    characterMovementComponent->SetInitialSpeed(6.0f);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);
    rotationComponent->SetRotateTime(0.1f); // 回転を速くする

    auto uiManager = GetOwnerScene()->GetUIManager();
    // ダッシュの狙いを表示する矢印のUIコンポーネントを追加
    for (int i = 0; i < _countof(arrowComponents); i++)
    {
        arrowComponents[i] = std::make_unique<UIArrowComponent>("./Data/Textures/ScissorsUI/Arrow.png", "dashAimArrow");
        arrowComponents[i]->SetWorldPosition({ 0.0f, 0.0f });
        arrowComponents[i]->SetVisible(true);
        arrowComponents[i]->SetSize({ 284.0f, 68.0f });
        //arrowComponents[i]->SetSize({ 300.0f, 50.0f });
        arrowComponents[i]->SetPivot({ 0.0f, 0.5f }); // 矢印の根元をプレイヤーの位置に合わせる
        arrowComponents[i]->SetVisible(false);
        uiManager->Add(arrowComponents[i]);
    }

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
        // テクスチャを作成
        heartEmpty = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/heart_empty.png");
        heartFull = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/heart_full.png");
        heartHalf = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/heart_half.png");

        hp = 10;
        int heartCount = maxHp / 2;

        for (int i = 0; i < heartCount; i++)
        {
            float x = heartPos.x + i * 120.0f;
            float y = heartPos.y;

            //  枠
            auto frame = std::make_shared<UIImageComponent>("hpFrame");
            frame->SetTexture(heartEmpty);
            frame->SetWorldPosition({ x, y });
            frame->SetSize({ heartSize, heartSize });

            uiManager->Add(frame);
            heartFramesHpUiComponents.push_back(frame);

            //  中身（最初はフル）
            auto fill = std::make_shared<UIImageComponent>("hpFill");
            fill->SetTexture(heartFull);
            fill->SetWorldPosition({ x, y }); // 同じ位置に重ねる
            fill->SetSize({ heartSize, heartSize });
            fill->zOrder = 2;

            uiManager->Add(fill);
            heartFillsHpUiComponents.push_back(fill);
        }
    }
    // 軌跡初期化
    trail.Initialize();

    // ダッシュIDの初期化
    dashSerial = 0;

    // 死亡演出の初期化
    startDeathPerform = false;
}

void ScissorsPlayer1::Update(float deltaTime)
{
    // ライフボーナスを記録
    ScoreSystem::AddLifeBonus(hp);

    auto scene = GetOwnerScene();
    if (scene->IsPaused())
    {// ポーズ中は入力を受け付けない 歩行音も止める
        return;
    }

    auto gameManager = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsGameManager>();

    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorOfType<TutorialActor>())
    {// チュートリアルだったら
        hp = std::max<int>(hp, 2);
        UpdateHpUI();
    }
    else
    {
#if 1
        if (hp <= 0 && !startDeathPerform)
        {// playerが死亡したら
            GetStateMachine()->ChangeState("Death");
            startDeathPerform = true;
            if (gameManager)
            {
                gameManager->EndGame(true);
            }
        }
#endif // 0
    }

    if (damageCooldownTimer > 0.0f)
    {// ダメージクールダウン中は無敵
        damageCooldownTimer -= deltaTime;
#if 0
        // 点滅処理
        blinkTimer += deltaTime;
        if (blinkTimer >= blinkInterval)
        {
            blinkTimer = 0.0f;
            isBlinkOn = !isBlinkOn;
        }

        // cpuColor反映
        if (isBlinkOn)
        {
            skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1, 0, 0, 1 }; // 赤
        }
        else
        {
            skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1, 1, 1, 1 }; // 白
        }
#else
        blinkTimer += deltaTime;

        float t = sinf(blinkTimer * 20.0f) * 0.5f + 0.5f;

        // 赤フラッシュ
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor =
        {
            1.0f,
            t,
            t,
            1.0f
        };

#endif // 0
    }
    else
    {
        // 無敵終了 → 色戻す
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1, 1, 1, 1 };
    }

    if (postDashInvincibleTimer > 0.0f)
    {// ダッシュ後のダメージクールダウン中は無敵
        postDashInvincibleTimer -= deltaTime;
    }

    if (knockBackTimer > 0.0f)
    {// ノックバック中のは無敵
        knockBackTimer -= deltaTime;
    }

    Character::Update(deltaTime);

    //if (gameManager)
    //{
    //    if (!gameManager->IsGameInputEnabled())
    //    {
    //        //stateMachine_->ChangeState("Idle");
    //        characterMovementComponent->SetSpeed(0.0f);
    //        return;
    //    }
    //}

    //　軌跡更新
    trail.UpdateTrail(deltaTime);

    // 入力に基づいて移動と回転を更新
    auto intent = inputComponent->GetIntent();

    if (auto gameManagerActor = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsGameManager>())
    {
        if (!gameManagerActor->IsGameInputEnabled())
        {
            if (!startDeathPerform)
            {
                stateMachine_->ChangeState("Idle");
            }
            intent = {};
        }
        else
        {

        }
    }



    // 左スティック入力
    float stickX = intent.leftMove.x;
    float stickZ = intent.leftMove.z;

    moveDir.x = stickX;
    moveDir.z = stickZ;
    auto state = stateMachine_->GetStateName();

    if (state != "Dash")
    {
        rotationComponent->SetDirection(GetLookDirection());
    }

    XMFLOAT3 pos = GetPosition();
#if 1
    {// ステージ外に出ないようにクランプ
        pos.x = std::clamp(pos.x, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX);
        pos.z = std::clamp(pos.z, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ);
        pos.y = 0.0f;
    }
#endif // 0
    SetPosition(pos);

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

        // これでダッシュの方向や溜めの強さを決める
        currentAimData = GetAimData(intent, deltaTime);

        if (currentAimData.isValid)
        {
            lastValidAimData = currentAimData;
        }

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

#if 0
    // ダッシュの狙いを表示する矢印のUIを更新
    XMFLOAT2 uiPos = WorldToUI(pos);
    if (dashAimArrowComponent)
    {
        dashAimArrowComponent->SetWorldPosition({ uiPos.x, uiPos.y });
    }

#endif // 0

    // playerの当たり判定をデバッグ表示
    DebugRender::DrawSphere(sphereComponent->GetComponentLocation(), playerRadius, debugPlayerCollisionColor, 0, true);
    // ダッシュ攻撃の当たり判定をデバッグ表示
    DebugRender::DrawSphere(dashAttackBox->GetComponentLocation(), dashAttackRange, debugDashCollisionColor, 0, true);
    //DebugRender::DrawBox(dashAttackBox->GetComponentLocation(), { dashAttackRange,height ,dashAttackRange }, debugDashCollisionColor, 0, true);
    debugPlayerCollisionColor = { 1,1,1,1 };
}

void ScissorsPlayer1::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    Character::DrawImGuiDetails();
    if (ImGui::Button(U8("プレイヤーが死亡する")))
    {
        GetStateMachine()->ChangeState("Death");
    }

    ImGui::DragFloat(U8("ハートのサイズ"), &heartSize);
    ImGui::DragFloat2(U8("ハートの場所"), &heartPos.x);
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
    DirectX::XMFLOAT3 playerPos = GetPosition();

    // ゲームパッド
    if (isGamepad)
    {
#if 0// 操作入力反転
        float x = -intent.rightMove.x;
        float z = -intent.rightMove.y;
#else
        float x = intent.rightMove.x;
        float z = intent.rightMove.y;
#endif // 1 // 操作入力反転

        float len = sqrt(x * x + z * z);

        static XMFLOAT3 lastDir = { 0,0,1 };

#if 0
        if (len > 0.4f)
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
#else
        static XMFLOAT3 smoothDir = { 0,0,1 };

        float deadZone = 0.25f;   // ← 小さい入力無視（手ブレ防止）
        float smoothSpeed = 15.0f; // ← 反応速度（大きいほどキビキビ）

        if (len > deadZone)
        {
            XMFLOAT3 targetDir = { x / len, 0.0f, z / len };

            //  補間
            smoothDir.x += (targetDir.x - smoothDir.x) * std::clamp(deltaTime * smoothSpeed, 0.0f, 1.0f);
            smoothDir.z += (targetDir.z - smoothDir.z) * std::clamp(deltaTime * smoothSpeed, 0.0f, 1.0f);

            // 正規化
            float l = sqrt(smoothDir.x * smoothDir.x + smoothDir.z * smoothDir.z);
            if (l > 0.0001f)
            {
                smoothDir.x /= l;
                smoothDir.z /= l;
            }

            // チャージ開始と継続
            isCharging = true;
            chargeTime += deltaTime;
            chargeTime = std::min<float>(chargeTime, maxChargeTime);

            lastDir = smoothDir;
        }
        else
        {
            // 入力弱いときは前の方向維持
            smoothDir = lastDir;
        }

        aim.dir = smoothDir;
#endif // 0

#if 0
        aim.power = chargeTime / maxChargeTime;
#else
        float t = chargeTime / maxChargeTime;
        t = t * t;
        aim.power = t;
#endif // 0
        //aim.power = lastStickPower;
        aim.isValid = true;
    }
    // マウス
    else
    {
#if 0
        // 押した瞬間
        if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Trigger))
        {
            DirectX::XMFLOAT2 cursor;
            if (InputSystem::GetMousePositionUI(cursor))
            {
                HitResultWithActor result;
                if (CollisionFunction::RaycastFromMouse(
                    cursor,
                    result,
                    CollisionHelper::ToBit(CollisionLayer::Floor)))
                {
                    dragStartWorld = result.hitPoint;
                    isDragging = true;
                    chargeTime = 0.0f;
                }
            }
        }
        if (isDragging && InputSystem::GetInputState("MouseLeft"))
        {
            DirectX::XMFLOAT2 cursor;
            if (!InputSystem::GetMousePositionUI(cursor))
                return lastValidAimData;

            HitResultWithActor result;
            if (!CollisionFunction::RaycastFromMouse(
                cursor,
                result,
                CollisionHelper::ToBit(CollisionLayer::Floor)))
                return lastValidAimData;

            DirectX::XMFLOAT3 current = result.hitPoint;

            // 引っ張りベクトル
            DirectX::XMFLOAT3 dragVec =
            {
                current.x - dragStartWorld.x,
                0,
                current.z - dragStartWorld.z
            };

            float len = sqrt(dragVec.x * dragVec.x + dragVec.z * dragVec.z);

            if (len > 0.001f)
            {
                // 逆方向に飛ぶ
                aim.dir =
                {
                    -dragVec.x / len,
                    0,
                    -dragVec.z / len
                };

                // 距離＝パワー
                float maxDrag = 5.0f;
                float power = std::clamp(len / maxDrag, 0.0f, 1.0f);

                aim.power = power;
                aim.isValid = true;
            }

            chargeTime += deltaTime;
            chargeTime = std::min<float>(chargeTime, maxChargeTime);
        }
        if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Release))
        {
            isDragging = false;
        }

#else// マウスが操作　ステージにあった方向に向く
        HitResultWithActor result;

        if (!InputSystem::GetInputState("MouseLeft"))
            return lastValidAimData;

        DirectX::XMFLOAT2 cursor;
        if (!InputSystem::GetMousePositionUI(cursor))
            return aim;

        if (!CollisionFunction::RaycastFromMouse(
            cursor,
            result,
            CollisionHelper::ToBit(CollisionLayer::Floor)))
            return aim;

        if (InputSystem::GetInputState("ScissorsAction", InputStateMask::Press))
        {
            isCharging = true;
            chargeTime += deltaTime;
            chargeTime = std::min<float>(chargeTime, maxChargeTime);
        }
        if (isCharging)
        {
            //aim.dir = GetForward();
            aim.dir = MathHelper::Normalize(MathHelper::Subtract(result.hitPoint, playerPos));
            aim.power = chargeTime / maxChargeTime;
            //aim.power = 1.0f;
            aim.isValid = true;
        }
#endif // 0
    }

    if (aim.isValid)
    {
        lastValidAimData = aim;
    }

    return aim;
}

// どの方向を向くか
DirectX::XMFLOAT3 ScissorsPlayer1::GetLookDirection() const
{
    if (useGamePad && stateMachine_->GetStateName() == "ChargeDash")
    {
        return lastValidAimData.dir;
    }
    else if (!useGamePad && (stateMachine_->GetStateName() == "ChargeDash" /*|| stateMachine_->GetStateName() == "Dash"*/))
    {
        return lastValidAimData.dir;
    }
    else
    {
        return moveDir;
    }
}

// ダメージを受けたときの処理
void ScissorsPlayer1::TakeDamage(int damage)
{
    if (damageCooldownTimer > 0.0f)
    {// ダメージ後のクールタイムを設定する
        return;
    }

#if 0 // 
    if (knockBackTimer > 0.0f)
    {// ノックバック後のクールタイムを設定する
        return;
    }

    if (postDashInvincibleTimer > 0.0f)
    {// ダッシュ後のクールタイムを設定する
        return;
    }
#endif // 0 // 

    damageCooldownTimer = damageCooldownInterval; // 無敵時間を設定

    blinkTimer = 0.0f;     // 点滅リセット
    isBlinkOn = true;      // 点滅開始

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
    ScoreSystem::ResetCombo();
}

void ScissorsPlayer1::RecoverHp(int recoverHp)
{
    Logger::Log(U8("回復前HP:") + std::to_string(hp));
    hp += recoverHp;
    hp = std::min<int>(hp, maxHp);
    Logger::Log(U8("プレイヤーHPを回復する HP:") + std::to_string(hp));
    // HPを表示するUIを更新
    UpdateHpUI();
}


// プレイヤーの攻撃処理
void ScissorsPlayer1::DoAttackHit()
{
    // 攻撃が当たった敵を記録するためのセットをクリア

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
#if 0
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
#endif // 0
}

// HPを表示するUIを更新する関数　
void ScissorsPlayer1::UpdateHpUI()
{
    for (int i = 0; i < heartFillsHpUiComponents.size(); i++)
    {
        int heartBase = i * 2;

        if (hp >= heartBase + 2)
        {
            // フル
            heartFillsHpUiComponents[i]->SetTexture(heartFull);
            heartFillsHpUiComponents[i]->SetVisible(true);
            heartFillsHpUiComponents[i]->SetSize({ heartSize, heartSize });

        }
        else if (hp == heartBase + 1)
        {
            // 半分
            heartFillsHpUiComponents[i]->SetTexture(heartHalf);
            heartFillsHpUiComponents[i]->SetVisible(true);
            heartFillsHpUiComponents[i]->SetSize({ heartSize, heartSize });
        }
        else
        {
            // 空 → 中身を消す（枠だけ残る）
            heartFillsHpUiComponents[i]->SetVisible(false);
            heartFillsHpUiComponents[i]->SetSize({ heartSize, heartSize });
        }
    }
}

// スコアポップアップを生成する関数
void ScissorsPlayer1::SpawnScorePopup(const DirectX::XMFLOAT3& pos, int score)
{
    Transform scoreTr{ pos,XMFLOAT3{0,0,0},XMFLOAT3{1,1,1} };
    auto popup = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ScorePopupActor>("ScorePopup", scoreTr);
    popup->SetScore(score);
}

//　ダッシュを使用する関数　これを呼ぶとダッシュの残り回数が減る
void ScissorsPlayer1::UseDash()
{
#if 0
    if (dashCount > 0)
    {
        dashCount--;
    }
    Logger::Log("Dash used. Remaining dash count: " + std::to_string(dashCount));

#endif // 0
    dashSerial++;
    chargeTime = 0.0f;

}

// ダッシュ可能かどうかを取得する関数
bool ScissorsPlayer1::CanDash() const
{
    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorOfType<TutorialActor>())
    {// チュートリアルだったら
        if (auto currentStep = tutorialActor->GetTutorialManager()->GetCurrentState())
        {
            return currentStep->CanDash();
        }
    }
    return dashCount > 0;
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


// 反射キルを適用する
void ScissorsPlayer1::ResolveReflectedKills()
{
    std::unordered_set<EnemyBase*> unique;
    int reflectedKillCount = 0;

    for (auto& hit : dashHits)
    {
        auto enemy = hit.enemy;

        if (!enemy) continue;
        if (unique.count(enemy)) continue;

        unique.insert(enemy);

        if (!enemy->pendingDeath) continue;

        // ここで初めて吹っ飛ぶ
        enemy->CallDeath(true);

        reflectedKillCount++;

        //// 通常スコア　コンボ込み
        //auto data = enemy->GetScoreData();
        //int addScore = ScoreSystem::ProcessHit(data, true);

        //SpawnScorePopup(enemy->GetPosition(), addScore);
    }

    // 反射ボーナス
    int reflectionBonus = reflectedKillCount * 80;
    // ダッシュボーナス
    int dashBonus = (killedEnemyCountInDash / 5) * 500;
    if (reflectionBonus > 0)
    {// 反射攻撃で死んでいたら
        ScoreSystem::AddReflectionBonus(reflectionBonus);
        InputSystem::SetVibration(1.0f, 0.2f);
        Logger::Log("ReflectionBonus: " + std::to_string(reflectionBonus));

        if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorOfType<TutorialActor>())
        {// チュートリアルだったら
            if (auto currentStep = tutorialActor->GetTutorialManager()->GetCurrentState())
            {
                if (currentStep->GetName() == "TutorialStep_AttackEnemyRedirect")
                {// 縫い返りを確認するステートで
                    currentStep->SetRedirectKillEnemy(true);
                }
            }
        }

    }
    if (dashBonus > 0)
    {// 5体以上
        ScoreSystem::AddDashBonus(dashBonus);
        SpawnBonusCoinBurst();
        Logger::Log("DashBonus: " + std::to_string(dashBonus));

        if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorOfType<TutorialActor>())
        {// チュートリアルだったら
            if (auto currentStep = tutorialActor->GetTutorialManager()->GetCurrentState())
            {
                if (currentStep->GetName() == "TutorialStep_AttackAllEnemy")
                {// 5体以上を確認するステートで
                    currentStep->SetBonusKill5Enemy(true);
                }
            }
        }


    }


    dashHits.clear();
}

// ダッシュ中の攻撃処理
void ScissorsPlayer1::AttackDash(EnemyBase* enemy)
{
    if (!enemy)
    {
        Logger::Warning(U8("エネミーがnullptrです"));
        return;
    }

    if (enemy->IsDead())
    {// 敵が死亡している場合
        return;
    }


    // 同じセグメントなら無視
    if (enemy && enemy->lastHitSegment != currentSegment)
    {
        enemy->lastHitSegment = currentSegment;

        bool isReflected = (currentSegment > 0);

        if (bool isKilled = enemy->OnHitByDash(isReflected))
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/enemyHit_strong.wav", 1.0f);

            if (isReflected)
            {
                dashHits.push_back({ enemy, true });
            }
#if 0
            else
            {
                auto data = enemy->GetScoreData();
                int addScore = ScoreSystem::ProcessHit(data, true);
                SpawnScorePopup(enemy->GetPosition(), addScore);
            }

#endif // 0

            killedEnemyCountInDash++;
            hitStopTimer = hitStopDuration;
        }
    }
}


// 周囲の敵を非表示
void ScissorsPlayer1::HideNearByRadius(float radius)
{
    auto actorManager = GetOwnerScene()->GetActorManager();
    if (!actorManager)
        return;

#if 1
    XMFLOAT3 playerPos = GetPosition();

    auto enemies = actorManager->GetActorsOfType<EnemyBase>();

    for (auto& enemy : enemies)
    {
        if (!enemy || enemy->IsDead())
        {
            continue;
        }

        XMFLOAT3 enemyPos = enemy->GetPosition();

        float distanceSq = MathHelper::DistanceSq(enemyPos, playerPos);

        if (distanceSq < radius * radius)
        {
            enemy->skeletalMeshComponent->SetIsVisible(false);
            enemy->skeletalMeshComponent->SetIsCastShadow(false);
        }
    }
#endif // 0
}

// 星を生成する
void ScissorsPlayer1::SpawnStarParticle(DirectX::XMFLOAT3 pos, XMFLOAT3 playerForward)
{
    // 星のテクスチャを生成
    DirectX::XMFLOAT2 uiPos = WorldToUI(pos);
    //auto starTex = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/star.png","star");
    auto starTex = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png", uiPos);
    //auto starTex = std::make_shared<UIDashEffect>("./Data/Textures/ScissorsUI/star.png", uiPos, DirectX::XMFLOAT2{ playerForward.x,playerForward.z });
    auto uiManager = GetOwnerScene()->GetUIManager();
    starTex->SetWorldPosition(uiPos);
    starTex->SetVisible(true);
    starTex->SetSize({ 50.0f, 50.0f });
    starTex->SetPivot({ 0.5f, 0.5f });
    uiManager->Add(starTex);
}

// ボーナスコインを生成する関数
void ScissorsPlayer1::SpawnBonusCoinBurst()
{
    auto pos = GetPosition();
    auto scene = GetOwnerScene();
    int coinCount = 1;

    Transform tr(pos, { 0,0,0 }, { 1.0f,1.0f,1.0f }); // ←少し大きい
    auto bonusUiActor = scene->GetActorManager()
        ->CreateAndRegisterActorWithTransform<BonusUiActor>("bonusCoin", tr);

    for (int i = 0; i < coinCount; i++)
    {
        float angle = static_cast<float>(i) / coinCount * DirectX::XM_2PI;

        DirectX::XMFLOAT3 offset =
        {
            cosf(angle) * 1.5f,
            0.5f,
            sinf(angle) * 1.5f
        };

        XMFLOAT3 coinPos = MathHelper::Add(pos, offset);

        Transform tr(coinPos, { 0,0,0 }, { 1.0f,1.0f,1.0f }); // ←少し大きい

        auto coin = scene->GetActorManager()
            ->CreateAndRegisterActorWithTransform<ButtonCoinActor>("bonusCoin", tr);

        coin->StartPerform(true); //  ボーナス指定
    }
}