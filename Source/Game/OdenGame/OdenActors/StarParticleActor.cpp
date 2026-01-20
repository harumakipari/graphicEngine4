#include "pch.h"

#include "StarParticleActor.h"

#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"


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

    const auto scene = GetOwnerScene();

    for (int i = 0; i < StarCount; ++i)
    {
        auto star = std::make_shared<UIImageComponent>(
            "./Data/Textures/star.png", "star_texture");

        star->SetPivot({ 0.5f,0.5f });
        star->SetSize({ 120, 120 });
        star->zOrder = 100;

        scene->GetUIManager()->Add(star);

        starTextures[i] = star;

        // 初期角度を120度ずつずらす
        starAngles[i] = XM_2PI * (i / float(StarCount));
    }


}

void StarParticleActor::Finalize()
{
    for (auto starTex : starTextures)
    {
        starTex->MarkPendingKill();
    }

    for (auto trailStar : trailStars)
    {
        trailStar.sprite->MarkPendingKill();
    }
}

void StarParticleActor::Update(float elapsedTime)
{
    time += elapsedTime;
    // スコアActor取得
    const auto scoreActor =
        GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIScoreViewActor");
    if (!scoreActor) return;

    scorePos = scoreActor->GetPosition();

    if (phase == StarPhase::Orbit)
    {
        float t = time / orbitDuration;
        angle = t * XM_2PI * 1.0; // 1周だけ

        for (int i = 0; i < StarCount; ++i)
        {
            float a = angle + starAngles[i];

            XMFLOAT3 offset =
            {
                cosf(a) * radius,
                sinf(a) * radius,
                0.0f
            };

            XMFLOAT3 pos =
            {
                startPos.x + offset.x,
                startPos.y + offset.y,
                startPos.z
            };

            XMFLOAT2 uiPos = WorldToUI(pos);
            starTextures[i]->SetWorldPosition(uiPos);
            starTextures[i]->SetWorldAngleDegree(XMConvertToDegrees(a));
        }

        if (time > orbitDuration)
        {
            phase = StarPhase::Merge;
            time = 0.0f;
        }
    }

    if (phase == StarPhase::Merge)
    {
        float t = std::clamp(time / mergeDuration, 0.0f, 1.0f);

        for (int i = 0; i < StarCount; ++i)
        {
            float a = angle + starAngles[i];
            float r = radius * (1.0f - t * 0.5f); // 30%位は残す

            XMFLOAT3 pos =
            {
                startPos.x + cosf(a) * r,
                startPos.y + sinf(a) * r,
                startPos.z
            };

            XMFLOAT2 uiPos = WorldToUI(pos);
            starTextures[i]->SetWorldPosition(uiPos);
        }

        if (t >= 1.0f)
        {
            phase = StarPhase::Attract;
            time = 0.0f;
        }
    }

    if (phase == StarPhase::Attract)
    {
        float t = std::clamp(time / attractDuration, 0.0f, 1.0f);
        float easeT = 1.0f - powf(1.0f - t, 3.0f);

        XMVECTOR s = XMLoadFloat3(&startPos);
        XMVECTOR e = XMLoadFloat3(&scorePos);
        XMVECTOR p = XMVectorLerp(s, e, easeT);

        XMFLOAT3 finalPos;
        XMStoreFloat3(&finalPos, p);

        // ===== 残り香判定 =====
        XMFLOAT3 d =
        {
            finalPos.x - prevTrailPos.x,
            finalPos.y - prevTrailPos.y,
            0.0f
        };

        float dist = sqrtf(d.x * d.x + d.y * d.y);

        if (dist > 1.0f)   // ★ この値が密度
        {
            SpawnTrailStar(prevTrailPos);
            prevTrailPos = finalPos;
            Logger::Log(U8("トレイル星をスポーンさせた"));

        }
        // =====================

        SetPosition(finalPos);

        XMFLOAT2 uiPos = WorldToUI(finalPos);
        starTextures[0]->SetWorldPosition(uiPos);

        starTextures[1]->SetVisible(false);
        starTextures[2]->SetVisible(false);

        if (t >= 1.0f)
        {
            MarkPendingKill();
        }
    }

    for (auto it = trailStars.begin(); it != trailStars.end(); )
    {
        it->life -= elapsedTime;

        float t = it->life / 0.3f;
        t = std::clamp(t, 0.0f, 1.0f);

        it->sprite->SetColor({ 1.0f,1.0f,1.0f,t });
        it->sprite->SetScale({ t * 0.4f, t * 0.4f });

        if (it->life <= 0.0f)
        {
            it->sprite->SetVisible(false);
            it = trailStars.erase(it);
        }
        else
        {
            ++it;
        }
    }


}

void StarParticleActor::StartParticle()
{
    phase = StarPhase::Orbit;
    time = 0.0f;
    angle = 0.0f;

    startPos = GetPosition();
    prevTrailPos = startPos;

    for (int i = 0; i < StarCount; ++i)
    {
        starTextures[i]->SetVisible(true);
    }

    particleComp->Play();
}

void StarParticleActor::SpawnTrailStar(const XMFLOAT3& worldPos)
{
    auto s = std::make_shared<UIImageComponent>("./Data/Textures/star.png", "trail_star");

    s->SetPivot({ 0.5f,0.5f });

    s->SetSize({ 120, 120 });
    float scale = MathHelper::RandomRange(1.2f, 1.8f);
    s->SetScale({ scale, scale });

    XMFLOAT2 uiPos = WorldToUI(worldPos);
    s->SetWorldPosition(uiPos);
    s->zOrder = 90;
    s->SetWorldAngleDegree(MathHelper::RandomRange(0.0f, 360.0f));
    s->SetColor({ 1.0f,1.0f,1.0f,0.8f });
    GetOwnerScene()->GetUIManager()->Add(s);

    float lifeTime = MathHelper::RandomRange(0.3f, 0.8f);

    trailStars.push_back({ s, lifeTime });
}