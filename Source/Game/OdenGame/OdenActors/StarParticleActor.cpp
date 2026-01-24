#include "pch.h"

#include "StarParticleActor.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenManagers/OdenGameManager.h"
#include "Game/OdenGame/OdenGameSession.h"
#include "Physics/CollisionFunction.h"


void StarParticleActor::Initialize(const Transform& transform)
{
    std::string parentName = "StarParticleActor_Root";
#if 0
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

#endif // 0

    startPos = GetPosition();

    const auto scene = GetOwnerScene();

    for (int i = 0; i < StarCount; ++i)
    {
        auto star = std::make_shared<UIImageComponent>(
            "./Data/Textures/starClear.png", "star_texture");

        star->SetWorldPosition({ -100.0f,-100.0f });
        star->SetPivot({ 0.5f,0.5f });
        star->SetSize({ 120, 120 });
        float scale = MathHelper::RandomRange(0.6f, 0.8f);
        star->SetScale({ scale,scale });
        star->zOrder = 100;
        star->SetVisible(false);

        scene->GetUIManager()->Add(star);

        starTextures[i] = star;

        // 初期角度を120度ずつずらす
        starAngles[i] = XM_2PI * (i / float(StarCount));
    }

    for (int i = 0; i < StarCount; ++i)
    {
        attractInfos[i].localOffset =
        {
            MathHelper::RandomRange(-40.0f, 40.0f),
            MathHelper::RandomRange(-40.0f, 40.0f),
            0.0f
        };

        attractInfos[i].speedFactor =
            MathHelper::RandomRange(0.6f, 1.5f);
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
        angle = t * XM_2PI * 0.6f; // 0.4周くらい
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
            starTextures[i]->SetVisible(true);
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
            starTextures[i]->SetVisible(true);

        }

        if (t >= 1.0f)
        {
            for (int i = 0; i < StarCount; ++i)
            {
                attractStartPos[i] = starTextures[i]->GetWorldPosition();
            }
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

        float trailFade = 1.0f - easeT;   // 終盤ほど小さく
        trailFade = std::clamp(trailFade, 0.2f, 1.0f);

        float dist = sqrtf(d.x * d.x + d.y * d.y);

        if (dist > trailInterval)
        {
            const int spawnCount = MathHelper::RandomRange(2, 4); // ★ここ

            for (int i = 0; i < spawnCount; ++i)
            {
                XMFLOAT3 offset =
                {
                    MathHelper::RandomRange(-trailSpread, trailSpread),
                    MathHelper::RandomRange(-trailSpread, trailSpread),
                    0.0f
                };

                XMFLOAT3 spawnPos =
                {
                    prevTrailPos.x + offset.x,
                    prevTrailPos.y + offset.y,
                    prevTrailPos.z
                };

                SpawnTrailStar(spawnPos, trailFade);
                //Logger::Log(U8("トレイル星をスポーンさせた"));
            }

            prevTrailPos = finalPos;
        }

        // =====================

        for (int i = 0; i < StarCount; ++i)
        {
            float localT = std::clamp(
                time / (attractDuration * attractInfos[i].speedFactor),
                0.0f, 1.0f);

            float easeT = 1.0f - powf(1.0f - localT, 3.0f);

            XMFLOAT2 s = attractStartPos[i];
            XMFLOAT2 scoreUI = WorldToUI(scorePos);

            XMFLOAT2 endOffset =
            {
                attractInfos[i].localOffset.x * 0.15f,
                attractInfos[i].localOffset.y * 0.15f
            };

            XMFLOAT2 e =
            {
                scoreUI.x + endOffset.x,
                scoreUI.y + endOffset.y
            };

            XMFLOAT2 p =
            {
                s.x + (e.x - s.x) * easeT,
                s.y + (e.y - s.y) * easeT
            };

            XMFLOAT2 final =
            {
                p.x + attractInfos[i].localOffset.x * (1.0f - easeT),
                p.y + attractInfos[i].localOffset.y * (1.0f - easeT)
            };
            float alpha = 1.0f - easeT;

            // 消え際を少し早めたいなら
            alpha = powf(alpha, 1.5f);

            // 最低限の残り防止
            alpha = std::clamp(alpha, 0.0f, 1.0f);

            starTextures[i]->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
            starTextures[i]->SetWorldPosition(final);
        }
        if (t >= 1.0f)
        {
            // ここでスコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    // スコアを加算する
                    gameManager->AddScore(pendingScore);
                    OdenGameSession::Instance().totalScore = gameManager->GetTotalScore();

                    // 満足度を加算する
                    //gameManager->AddSatisfaction(score.satisfaction);
                }
            }
            MarkPendingKill();
        }

        for (auto it = trailStars.begin(); it != trailStars.end(); )
        {
            it->life -= elapsedTime;

            float t = it->life / 0.3f;
            t = std::clamp(t, 0.0f, 1.0f);

            it->sprite->SetColor({ 1.0f,1.0f,1.0f,t });
            it->sprite->SetScale({ t * 0.6f, t * 0.6f });

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
}


void StarParticleActor::StartParticle(const int score)
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

    pendingScore = score;
    //particleComp->Play();
}

void StarParticleActor::SpawnTrailStar(const XMFLOAT3& worldPos, float intensity)
{
    auto s = std::make_shared<UIImageComponent>("./Data/Textures/starClear.png", "trail_star");

    s->SetPivot({ 0.5f,0.5f });

    s->SetSize({ 120, 120 });
    float scale = MathHelper::RandomRange(1.8f, 2.5f) * intensity;
    s->SetScale({ scale, scale });

    XMFLOAT2 uiPos = WorldToUI(worldPos);
    s->SetWorldPosition(uiPos);
    s->zOrder = 90;
    //    s->SetWorldAngleDegree(MathHelper::RandomRange(0.0f, 360.0f));
    s->SetColor({ 1.0f,1.0f,1.0f,0.8f * intensity });
    GetOwnerScene()->GetUIManager()->Add(s);

    float lifeTime = MathHelper::RandomRange(0.3f, 0.5f);

    trailStars.push_back({ s, lifeTime });
}