#include "pch.h"
#include "ButtonCoinActor.h"

#include <algorithm>

#include "Game/Scenes/GameScene.h"
#include "Physics/CollisionFunction.h"
#include "UI/Widgets/Widget.h"

void ButtonCoinActor::Initialize(const Transform& transform)
{
    std::string parentName = "buttonCoin";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/NormalButtonCoin.glb");
    skeletalMeshComponent->SetIsCastShadow(false); // 
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";

    particleComponent = AddComponent<ParticleComponent>("particleComponent", parentName);
    particleComponent->Load("./Data/Effect/Files/ScissorsGameCoinAppearEffect.json");
    //particleComponent->Load("./Data/Effect/Files/ScissorsGameCoinBurstEffect.json");

    // 開始位置を設定
    startPos = transform.GetLocation();
}


void ButtonCoinActor::Update(float deltaTime)
{
    // 調整のために
    auto scene = static_cast<GameScene*>(GetOwnerScene());
    auto& tuning = scene->coinTuning;
    float trailSpawnInterval = tuning.trailSpawnInterval;
    float trailSize = tuning.trailSize;
    float duration = tuning.duration;
    float height = tuning.height;


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
        {
            skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = 1.0f;
            skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 10.0f;
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
        //MarkPendingKill();
        break;
    }



}

void ButtonCoinActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("演出開始")))
    {
        StartPerform();
    }
#endif
}

// 演出開始する
void ButtonCoinActor::StartPerform()
{
    state = CoinState::Rising; // 上昇中に変更
    skeletalMeshComponent->SetIsVisible(true);
    elapsedTime = 0.0f;
    SetPosition(startPos);
    particleComponent->Play();

    // 発光値をリセットする
    skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = 0.0f;

    // 星を生成する位置
    DirectX::XMFLOAT3 starPos = startPos;
    starPos.y += 0.2f;
    DirectX::XMFLOAT2 screenPos = WorldToUI(starPos);

    auto uiManager = GetOwnerScene()->GetUIManager();
    // 星を生成する
    for (int i = 0; i < 4; i++)
    {
        auto star = std::make_shared<UIStarEffect>("./Data/Textures/ScissorsUI/star.png", screenPos);
        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }
}

// きらきらバースト
void ButtonCoinActor::SpawnBurst()
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    auto pos = GetPosition();
    auto screenPos = WorldToUI(pos);

    int count = 8;

    for (int i = 0; i < count; i++)
    {
        float baseAngle = static_cast<float>(i) / count * DirectX::XM_2PI;
        float randomOffset = MathHelper::RandomRange(-0.2f, 0.2f);
        float angle = baseAngle + randomOffset;

        float speed = MathHelper::RandomRange(300.0f, 600.0f);

        auto star = std::make_shared<UIStarBurstEffect>(
            "./Data/Textures/ScissorsUI/starClear.png",
            screenPos);

        // velocityを外からセットするようにする
        star->SetVelocity({
            cosf(angle) * speed,
            sinf(angle) * speed
            });

        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }
}