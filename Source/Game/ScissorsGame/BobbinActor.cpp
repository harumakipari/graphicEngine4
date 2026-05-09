#include "pch.h"
#include "BobbinActor.h"

#include "BossSpawner.h"
#include "ButtonCoinActor.h"
#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
#include "ScissorsPlayer1.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void BobbinActor::Initialize(const Transform& transform)
{
    std::string parentName = "BobbinActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/BobbinModel.glb", false, true);

    bobbinApplyRangeMeshComponent = AddComponent<SkeletalMeshComponent>("applyRangeModel", parentName);
    bobbinApplyRangeMeshComponent->SetModel("./Data/TeamModels/Marks/BobbinApplyRange.gltf", false, true);
    bobbinApplyRangeMeshComponent->overrideDeferredPipelineName = "OpaqueMarkPS";
    bobbinApplyRangeMeshComponent->SetIsCastShadow(false);
    bobbinApplyRangeMeshComponent->SetRelativeScaleDirect({ 0.0f,0.0f,0.0f });
    //bobbinApplyRangeMeshComponent->SetIsVisible(false);

    bobbinStringMeshComponent = AddComponent<SkeletalMeshComponent>("bobbinStringModel", parentName);
    bobbinStringMeshComponent->SetModel("./Data/TeamModels/Item/BobbinStringModel.gltf", false, true);
    bobbinStringMeshComponent->SetIsCastShadow(false);
    bobbinStringMeshComponent->SetRelativeScaleDirect({ 0.0f,0.0f,0.0f });
    bobbinStringMeshComponent->SetRelativeLocationDirect({ 0.0f,1.45f,0.0f });

    DirectX::XMFLOAT3 size = { 1.9f,2.9f,1.9f };

    auto boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    // DirectX::XMFLOAT3 size = MathHelper::MultiplyF3XF3(skeletalMeshComponent->GetModelSize(), transform.GetScale());
    float mass = 0.0f;
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,size.y * 0.5f ,0.0f });
    boxComponent->SetMass(mass);
    boxComponent->SetLayer(CollisionLayer::Bobbin);
    //boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->Initialize();
    boxComponent->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            uint32_t mask = CollisionHelper::ToBit(CollisionLayer::PlayerWeapon);
            if (other->GetCollisionLayer() != mask)// ダッシュの当たり判定じゃなかったら、
                return;

            auto player = dynamic_cast<ScissorsPlayer1*>(other->GetOwner());
            if (!player) return;

            if (player->GetStateMachine()->GetStateName() == "Dash")
            {// playerが突進中だったら
                int currentDash = player->GetDashId();
                if (currentDash == lastUsedDashSerial)
                    return;

                lastUsedDashSerial = currentDash;

                UseBobbin();
            }
        }
    );
    bobbinState = BobbinState::CoolDown;

    // チャージ音のオーディオコンポーネント
    chargeAudioComponent = AddComponent<CoreAudioSourceComponent>("chargeAudioComponent", parentName);
    chargeAudioComponent->SetSource(L"./Data/Sound/SE1/bobbin_charge.wav");
    chargeAudioComponent->SetVolume(0.2f);
    chargeAudioComponent->SetLoop(true);
}

void BobbinActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 center = GetPosition();

    switch (bobbinState)
    {
    case BobbinState::CoolDown:
        cooldownTimer -= deltaTime;
        if (cooldownTimer < 0.0f)
        {
            bobbinState = BobbinState::Charging;
            chargeAudioComponent->Play();
        }
        break;
    case BobbinState::Charging:
    {
        chargeTimer += deltaTime;

        float t = chargeTimer / chargeTime;
        t = std::clamp(t, 0.0f, 1.0f);

        currentRadius = maxRadius * t;

        // 床の有効範囲のモデルのスケールを大きくする
        {
            float scale = applyRangeMaxScale * t;
            bobbinApplyRangeMeshComponent->SetRelativeScaleDirect({ scale,scale,scale });
        }

        // ボビンを回転させる
        {
            DirectX::XMFLOAT3 rot = skeletalMeshComponent->GetRelativeEulerRotation();
            rot.y += 360.0f * deltaTime; // 1秒で360度回転
            skeletalMeshComponent->SetRelativeEulerRotationDirect(rot);
        }
        // ボビンの糸のモデルのスケールを大きくする
        {
            float scale = std::lerp(0.6f, 1.0f, t);
            bobbinStringMeshComponent->SetRelativeScaleDirect({ scale,scale,scale });
        }
        if (t >= 1.0f)
        {
            bobbinState = BobbinState::ChargeEnd;
            chargeAudioComponent->Stop();
        }
    }
    break;
    case BobbinState::ChargeEnd:
    {
        bobbinApplyRangeMeshComponent->SetRelativeScaleDirect({ applyRangeMaxScale,applyRangeMaxScale,applyRangeMaxScale });

    }
    break;
    case BobbinState::Fired:
        ApplyToEnemies(center);
        Reset();
        break;
    }
    // ボビンの当たり判定をデバッグ表示
    DebugRender::DrawSphere(center, currentRadius, { 1,0,0.5f,1 }, 0, true);
}


void BobbinActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("ボビンを使用する")))
    {
        UseBobbin();
    }
    ImGui::DragFloat(U8("広がる最大半径"), &maxRadius, 0.5f);
    ImGui::DragFloat(U8("クールタイム"), &cooldownInterval, 0.5f);
    ImGui::DragFloat(U8("maxになるまでにかかる時間"), &chargeTime, 0.5f);

#endif
}

// ボビンのサイズを設定する
void BobbinActor::SetBobbinSize(BobbinSize bobbinSize)
{
    switch (bobbinSize)
    {
    case BobbinSize::Small:
        maxRadius = 3.0f; // 最大半径
        cooldownInterval = 0.1f;// クールタイム
        chargeTime = 3.5f; // 何秒でMaxになるか
        applyRangeMaxScale = 1.0f;
        break;
    case BobbinSize::Medium:
        maxRadius = 4.5f; // 最大半径
        cooldownInterval = 0.1f;// クールタイム
        chargeTime = 3.5f; // 何秒でMaxになるか
        applyRangeMaxScale = 1.5f;
        break;
    case BobbinSize::Big:
        maxRadius = 6.0f; // 最大半径
        cooldownInterval = 0.1f;// クールタイム
        chargeTime = 3.5f; // 何秒でMaxになるか
        applyRangeMaxScale = 2.0f;
        break;
    }
}

// ボビンを使用する
void BobbinActor::UseBobbin()
{
    if (bobbinState == BobbinState::Fired)
    {// 
        return;
    }
    // チャージ音を止める
    chargeAudioComponent->Stop();
    bobbinState = BobbinState::Fired;
    Logger::Log(U8("糸巻を使用した"));
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/bobbin_use.wav", 1.0f);
}

// ボビンをリセットする
void BobbinActor::Reset()
{
    currentRadius = 0.0f;
    chargeTimer = 0.0f;
    hitEnemies.clear();
    bobbinState = BobbinState::CoolDown;
    cooldownTimer = cooldownInterval;
    bobbinApplyRangeMeshComponent->SetRelativeScaleDirect({ 0.0f,0.0f,0.0f });
    bobbinStringMeshComponent->SetRelativeScaleDirect({ 0.0f,0.0f,0.0f });
}

// 敵を玉止めする
void BobbinActor::ApplyToEnemies(const DirectX::XMFLOAT3 center)
{
    std::vector<std::shared_ptr<ITieable>> candidates;

    auto waveManager = GetOwnerScene()->GetActorManager()->GetActorOfType<WaveManager>();

    // 敵を集める
    for (auto& w : waveManager->aliveEnemies)
    {
        if (auto e = w.lock())
        {
            if (!e->IsDead())
            {
                candidates.push_back(e);
            }
        }
    }

    if (auto bossSpawner = GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {// 
        // 敵を集める
        for (auto& w : bossSpawner->aliveEnemies)
        {
            if (auto e = w.lock())
            {
                if (!e->IsDead())
                {
                    candidates.push_back(e);
                }
            }
        }
    }


    auto boss = GetOwnerScene()->GetActorManager()->GetActorOfType<RabbitBossEnemyActor>();
    if (boss)
    {// ボスがいたら
        candidates.push_back(boss);
    }

    int index = 0;

    for (auto e : candidates)
    {
        auto actor = dynamic_cast<Actor*>(e.get());
        if (!actor) continue;

        auto enemyPos = actor->GetPosition();

        float dx = enemyPos.x - center.x;
        float dz = enemyPos.z - center.z;

        float distanceSq = dx * dx + dz * dz;

        float sumRadius = currentRadius + e->GetRadius();
        float radiusSq = sumRadius * sumRadius;

        if (distanceSq <= radiusSq)
        {
            if (e->IsTied())
            {
                if (auto boss = dynamic_cast<RabbitBossEnemyActor*>(actor))
                {// ボスの場合
#if 0
                    BossDamageContext bossDamageContext = {};
                    bossDamageContext.killedEnemyBeforeHitCount = 0;
                    bossDamageContext.baseDamage = 10.0f;
                    bossDamageContext.isBossStunned = boss->IsStunned();
                    float damage = boss->ComputeDamage(bossDamageContext);
#else
                    float damage = 0.0f;
#endif // 0
                    Logger::Log(U8("ボビンによる攻撃でボスに大ダメージ：") + std::to_string(damage));
                    boss->TakeDamage(static_cast<int>(damage));
                }
                // すでに玉止めされている → 死亡
                else if (auto enemy = dynamic_cast<EnemyBase*>(actor))
                {
                    enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
                    enemy->CallDeath(false); // 死亡演出開始処理
                    // 死亡演出に遅延を入れる
                    enemy->SetDelayBeforeKnockback(index * 0.08f);
                    index++;
                }
            }
            else
            {
                // 玉止めされていない → 玉止め
                e->OnTied();
            }
        }
    }


    // 複数ボーナス
    int dashBonus = (index / 5) * 500;
    if (dashBonus > 0)
    {// 5体以上
        ScoreSystem::AddBonusScore(dashBonus);
        SpawnBonusCoinBurst();
        Logger::Log("DashBonus: " + std::to_string(dashBonus));
    }


}

void BobbinActor::SpawnBonusCoinBurst()
{
    auto pos = GetPosition();
    auto scene = GetOwnerScene();
    int coinCount = 1;

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

        Transform tr(coinPos, { 0,0,0 }, { 1.0f,1.0f,1.0f });
        auto coin = scene->GetActorManager()
            ->CreateAndRegisterActorWithTransform<ButtonCoinActor>("bonusCoin", tr);

        coin->StartPerform(true); //  ボーナス指定
    }
}
