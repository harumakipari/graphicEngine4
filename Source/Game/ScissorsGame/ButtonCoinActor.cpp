#include "pch.h"
#include "ButtonCoinActor.h"

#include <algorithm>

#include "Game/Scenes/GameScene.h"
#include "Physics/CollisionFunction.h"
#include "UI/Widgets/Widget.h"

void ButtonCoinActor::Initialize(const Transform& transform)
{
    //particleComponent->Load("./Data/Effect/Files/ScissorsGameCoinBurstEffect.json");

    // 開始位置を設定
    startPos = transform.GetLocation();
}


void ButtonCoinActor::Update(float deltaTime)
{
    // 調整のために
    auto scene = static_cast<GameScene*>(GetOwnerScene());
    auto& tuning = isBonus ? scene->bonusCoin : scene->normalCoin;
    float trailSpawnInterval = tuning.trailSpawnInterval;
    float trailSize = tuning.trailSize;
    float duration = tuning.duration;
    float height = tuning.height;

    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT2 screePos = WorldToUI(position);

    for (auto star : starEffects)
    {// 星をコインに追従させるため
        star->SetFollowPos(screePos);
    }

    switch (state)
    {
    case CoinState::Rising:
    {
        elapsedTime += deltaTime;

        float t = elapsedTime / duration;
        t = std::min<float>(t, 1.0f);

        float yOffset = height * (1.0f - (1.0f - t) * (1.0f - t)); // easeOut

        auto pos = startPos;
        pos.y += yOffset;
        SetPosition(pos);

        float rotation = DirectX::XMConvertToDegrees(DirectX::XM_2PI * 1.0f * t); // 三回転
        SetEulerRotation({ 0.0f,rotation,0.0f });


        if (t >= 0.8f)
        {// フラッシュの値を設定
            //skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = 1.0f;
            //skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 8.5f;
        }

        if (t >= 1.0f)
        {
            //MarkPendingKill();
            skeletalMeshComponent->SetIsVisible(false);
            state = CoinState::Burst;
            // キラキラを出す。
            SpawnBurst();
            t = 1.0f;
            elapsedTime = 0.0f;
        }

        // 上昇中だけ
        if (t < 1.0f)
        {// 軌跡を出す
            trailTimer += deltaTime;

            if (trailTimer >= trailSpawnInterval)
            {
                trailTimer = 0.0f;

                auto pos = GetPosition();
                pos.y -= 0.2f; // 少し下

                auto screenPos = WorldToUI(pos);

                auto star = std::make_shared<UIStarTrailEffect>("./Data/Textures/ScissorsUI/starClear.png", "starTrail");
                star->SetWorldPosition(screenPos);
                star->SetSize({ trailSize, trailSize });


                GetOwnerScene()->GetUIManager()->Add(star);
            }
        }
    }
    break;
    case CoinState::Burst:
        elapsedTime += deltaTime;
        if (elapsedTime >= 1.0f)
        {
            state = CoinState::Finished;
        }
        break;
    case CoinState::Finished:
        MarkPendingKill();
        break;
    }

}

void ButtonCoinActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("演出開始")))
    {
        StartPerform(true);
    }
#endif
}

// 演出開始する
void ButtonCoinActor::StartPerform(bool isBonus)
{
    this->isBonus = isBonus;
    auto scene = static_cast<GameScene*>(GetOwnerScene());
    auto& tuning = isBonus ? scene->bonusCoin : scene->normalCoin;
    
    std::string parentName = "buttonCoin";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel(tuning.modelPath);
    skeletalMeshComponent->SetIsCastShadow(false); // 
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameCoinPS";

    particleComponent = AddComponent<ParticleComponent>("particleComponent", parentName);
    particleComponent->Load("./Data/Effect/Files/ScissorsGameCoinAppearEffect.json");


    state = CoinState::Rising; // 上昇中に変更
    skeletalMeshComponent->SetIsVisible(true);
    elapsedTime = 0.0f;
    SetPosition(startPos);


    // 音を再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/button_piro.wav", 1.f);

    //particleComponent->Play();

    // 発光値をリセットする
    skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = 0.0f;

    // 星を生成する位置
    DirectX::XMFLOAT2 screenPos = WorldToUI(startPos);

    auto uiManager = GetOwnerScene()->GetUIManager();
    int count = 4;
    for (int i = 0; i < count; i++)
    {
        float baseAngle = static_cast<float>(i) / count * DirectX::XM_2PI;
        float randomOffset = MathHelper::RandomRange(-0.5f, 0.5f);
        float angle = baseAngle + randomOffset;

        float speed = 150.0f;

        auto star = std::make_shared<UIStarEffect>(
            "./Data/Textures/ScissorsUI/star.png",
            screenPos);

        star->SetVelocity({
            cosf(angle) * speed,
            sinf(angle) * speed
            });

        star->SetSize({ 100,100 });
        star->SetFollowPos(screenPos);
        uiManager->Add(star);
        starEffects.push_back(star);
    }
}

void ButtonCoinActor::Finalize()
{
    
}

// きらきらバースト
void ButtonCoinActor::SpawnBurst()
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    auto pos = GetPosition();
    auto screenPos = WorldToUI(pos);

    auto scene = static_cast<GameScene*>(GetOwnerScene());
    auto& tuning = scene->normalCoin;
    int count = tuning.burstCount;


    for (int i = 0; i < count; i++)
    {
        float baseAngle = static_cast<float>(i) / count * DirectX::XM_2PI;
        float randomOffset = MathHelper::RandomRange(-0.2f, 0.2f);
        float angle = baseAngle + randomOffset;

        float baseSpeed = tuning.burstShrinkSpeed;
        float radiusRand = sqrt(MathHelper::RandomRange(0.0f, 1.0f));
        float speed = baseSpeed * radiusRand;

        auto star = std::make_shared<UIStarBurstEffect>(
            "./Data/Textures/ScissorsUI/starClear.png",
            screenPos);

        // velocityを外からセットするようにする
        star->SetVelocity({
            cosf(angle) * speed,
            sinf(angle) * speed
            });

        star->SetSize({ tuning.burstSize,tuning.burstSize });
        uiManager->Add(star);
    }
}