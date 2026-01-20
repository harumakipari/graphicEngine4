#include "pch.h"

#include "StarParticleActor.h"

#include "Engine/Scene/Scene.h"



void StarParticleActor::Initialize(const Transform& transform)
{
    std::string parentName = "StarParticleActor_Root";
    // 星のコンポーネントを追加
    particleComp = this->AddComponent<class ParticleComponent>("starParticleComponent");
    particleComp->Load("./Data/Effect/Files/starAttractEffect.json");
    particleComp->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
    };
    particleComp->SetAddSettings(settings);
    // イージング
    easingRunner = std::make_shared<EasingRunner>();

    particleComp->Play();

    startPos = GetPosition();
}

void StarParticleActor::Update(float elapsedTime)
{
    time += elapsedTime;

    // スコアActor取得
    const auto scoreActor =
        GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIScoreViewActor");
    if (!scoreActor) return;

    scorePos = scoreActor->GetPosition();

    float t = std::clamp(time / duration, 0.0f, 1.0f);

    // === イージング（超重要）===
    // 最初ゆっくり → 最後シュッ
    float easeT = 1.0f - powf(1.0f - t, 3.0f); // OutCubic

    // === 回転 ===
    angle += elapsedTime * XM_2PI * 2.5f; // 回転速度

    // 半径は徐々に小さく
    float currentRadius = radius * (1.0f - easeT);

    XMFLOAT3 swirlOffset =
    {
        cosf(angle) * currentRadius,
        sinf(angle * 1.3f) * currentRadius * 0.5f,
        sinf(angle) * currentRadius
    };

    // === 吸引（中心点がスコアに近づく）===
    XMVECTOR start = XMLoadFloat3(&startPos);
    XMVECTOR score = XMLoadFloat3(&scorePos);
    XMVECTOR center = XMVectorLerp(start, score, easeT);

    XMFLOAT3 centerPos;
    XMStoreFloat3(&centerPos, center);

    // === 最終位置 ===
    XMFLOAT3 finalPos =
    {
        centerPos.x + swirlOffset.x,
        centerPos.y + swirlOffset.y,
        centerPos.z + swirlOffset.z
    };

    SetPosition(finalPos);

    // 到達
    if (t >= 1.0f)
    {
        //particleComp->Stop();
        // ここでスコア加算SEとか
    }
}


void StarParticleActor::StartParticle()
{
    //TestEasingHandler handler;
    //handler.AddEasing(TestEaseType::OutCubic, 0.f, 1.0f, 0.4f);
    //handler.SetCompletedFunction([this]()
    //    {
    //        particleComp->Stop();
    //    });
    //PropertyAccessor<float> accessor;
    //accessor.getter = [this]() { return a; };
    //accessor.setter = [this](float t)
    //    {
    //        a = t;
    //        if (const auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIScoreViewActor"))
    //        {
    //            XMFLOAT3 pos = actor->GetPosition();
    //            XMVECTOR ScorePos = XMLoadFloat3(&pos);
    //            XMFLOAT3 currentPos = GetPosition();
    //            XMVECTOR StartPos = XMLoadFloat3(&currentPos);

    //            XMVECTOR NewPos = XMVectorLerp(StartPos, ScorePos, t);
    //            DirectX::XMFLOAT3 newPos;
    //            XMStoreFloat3(&newPos, NewPos);
    //            //particleComp->SetWorldLocationDirect(newPos);
    //            SetPosition(pos);
    //        }

    //    };

    //easingRunner->StartHandler(handler, accessor);
    //if (const auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIScoreViewActor"))
    //{
    //    XMFLOAT3 pos = actor->GetPosition();
    //    SetPosition(pos);
    //}

    //particleComp->Play();
}