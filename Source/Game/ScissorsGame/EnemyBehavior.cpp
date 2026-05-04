#include "pch.h"
#include "EnemyBehavior.h"
#include "EnemyBase.h"
#include "ScissorsGameState.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void StaticBehavior::Update(EnemyBase* e, float dt)
{
    e->Face({ 0.0f,0.0f,-1.0f });
}

void LinearBehavior::Update(EnemyBase* e, float deltaTime)
{
    DirectX::XMFLOAT3 pos = e->GetPosition();
    DirectX::XMFLOAT3 moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ステージ端で反転
    if (pos.x < ScissorsGameState::stageMinX || pos.x > ScissorsGameState::stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX);
    }

    if (pos.z < ScissorsGameState::stageMinZ || pos.z > ScissorsGameState::stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ);
    }

    e->SetPosition(pos);
    e->SetMoveDirection(moveDirection);

    e->Face(moveDirection);
}


void WaveHorizontalBehavior::Update(EnemyBase* e, float dt)
{
    waveTime += dt;

    auto pos = e->GetPosition();

    auto start = e->GetStartPosition();
    auto moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    start.x += moveDirection.x * speed * dt;

    e->SetStartPosition(start);

    // 波
    pos.x = start.x;
    pos.z = start.z + sin(waveTime * waveFrequency) * waveAmplitude;

    e->SetPosition(pos);

    if (pos.x < ScissorsGameState::stageMinX)
    {
        pos.x = ScissorsGameState::stageMinX;
        start.x = pos.x;
        moveDirection.x = 1.0f;
    }
    else if (pos.x > ScissorsGameState::stageMaxX)
    {
        pos.x = ScissorsGameState::stageMaxX;
        start.x = pos.x;
        moveDirection.x = -1.0f;
    }

    e->SetMoveDirection(moveDirection);
    e->Face(moveDirection);
}

void WaveVerticalBehavior::Update(EnemyBase* e, float dt)
{
    waveTime += dt;

    auto pos = e->GetPosition();

    auto start = e->GetStartPosition();
    auto moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    start.z += moveDirection.z * speed * dt;

    e->SetStartPosition(start);

    pos.z = start.z;
    pos.x = start.x + sin(waveTime * waveFrequency) * waveAmplitude;

    e->SetPosition(pos);

    if (pos.z < ScissorsGameState::stageMinZ)
    {
        pos.z = ScissorsGameState::stageMinZ;
        start.z = pos.z;
        moveDirection.z = 1.0f;
    }
    else if (pos.z > ScissorsGameState::stageMaxZ)
    {
        pos.z = ScissorsGameState::stageMaxZ;
        start.z = pos.z;
        moveDirection.z = -1.0f;
    }


    e->SetMoveDirection(moveDirection);
    e->Face(moveDirection);
}

void WaveMoveBehavior::Update(EnemyBase* e, float dt)
{
    waveTime += dt;

    auto pos = e->GetPosition();

    auto start = e->GetStartPosition();
    auto moveDir = e->GetMoveDirection();
    float speed = e->GetSpeed();

    // ----------- 正規化（超重要）-----------
    XMVECTOR dirVec = XMVector3Normalize(XMLoadFloat3(&moveDir));
    XMStoreFloat3(&moveDir, dirVec);

    // ----------- 進行方向に移動 -----------
    start.x += moveDir.x * speed * dt;
    start.z += moveDir.z * speed * dt;

    // ----------- 壁判定＆反射 -----------
    bool reflected = false;

    // X壁
    if (start.x < ScissorsGameState::stageMinX)
    {
        start.x = ScissorsGameState::stageMinX;
        moveDir.x *= -1.0f;
        reflected = true;
    }
    else if (start.x > ScissorsGameState::stageMaxX)
    {
        start.x = ScissorsGameState::stageMaxX;
        moveDir.x *= -1.0f;
        reflected = true;
    }

    // Z壁
    if (start.z < ScissorsGameState::stageMinZ)
    {
        start.z = ScissorsGameState::stageMinZ;
        moveDir.z *= -1.0f;
        reflected = true;
    }
    else if (start.z > ScissorsGameState::stageMaxZ)
    {
        start.z = ScissorsGameState::stageMaxZ;
        moveDir.z *= -1.0f;
        reflected = true;
    }

    // ----------- 直交ベクトル（波方向）-----------
    XMFLOAT3 sideDir = { -moveDir.z, 0.0f, moveDir.x };

    // ----------- 波適用 -----------
    float wave = sin(waveTime * waveFrequency) * waveAmplitude;

    pos.x = start.x + sideDir.x * wave;
    pos.z = start.z + sideDir.z * wave;
    pos.y = start.y;

    // ----------- 反映 -----------
    e->SetStartPosition(start);
    e->SetPosition(pos);
    e->SetMoveDirection(moveDir);
    e->Face(moveDir);
}

void ChaseBehavior::Update(EnemyBase* e, float dt)
{
    auto player = e->GetPlayer();
    if (!player) return;

    auto pos = e->GetPosition();
    auto target = player->GetPosition();

    // プレイヤーへの方向
    DirectX::XMFLOAT3 dir = MathHelper::Normalize(MathHelper::Subtract(target, pos));

    // 敵同士が重ならないように分離
    DirectX::XMFLOAT3 separation = { 0,0,0 };
    auto enemies = e->GetOwnerScene()->GetActorManager()->GetActorsOfType<EnemyBase>();

    for (auto& other : enemies)
    {
        if (other.get() == e) continue;

        auto otherPos = other->GetPosition();

        auto diff = MathHelper::Subtract(pos, otherPos);
        float distSq = diff.x * diff.x + diff.z * diff.z;

        if (distSq < avoidDist * avoidDist)
        {
            separation = MathHelper::Add(separation, diff);
        }
    }

    // 合成
    dir = MathHelper::Add(dir, MathHelper::Multiply(separation, separationWeight));
    dir = MathHelper::Normalize(dir);

    e->Move(dir, dt);
    e->Face(dir);
}

void RescueBehavior::Enter(EnemyBase* e)
{
    e->CreateScissorsVisual();

    wanderTarget =
    {
        MathHelper::RandomRange(e->rng, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX),
        0.0f,
        MathHelper::RandomRange(e->rng, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ)
    };
}

void RescueBehavior::Update(EnemyBase* e, float dt)
{
    // クールタイム中
    if (rescueCooldown > 0.0f)
    {
        rescueCooldown -= dt;

        // 予約してたら解除
        if (target && target->reservedBy == e)
        {
            target->reservedBy = nullptr;
        }
        target = nullptr;

        // 軽く徘徊させる
        UpdateWander(e, dt);

        return;
    }


    // ターゲットがない or 無効なら探す
    if (!target || target->IsDead() ||
        !(target->GetState() == EnemyBase::YarnState::Tied ||
            target->GetState() == EnemyBase::YarnState::Tying))
    {
        // 予約解除
        if (target && target->reservedBy == e)
        {
            target->reservedBy = nullptr;
        }

        target = FindTiedEnemy(e);
        e->rescueTimer = 0.0f;

        // ターゲットいない
        if (!target)
        {
            UpdateWander(e, dt);
            return;
        }

        // 予約
        target->reservedBy = e;
    }

    // ターゲットに向かう 
    auto pos = e->GetPosition();
    auto targetPos = target->GetPosition();

    DirectX::XMFLOAT3 dir = MathHelper::Normalize(MathHelper::Subtract(targetPos, pos));

    float len = MathHelper::Distance(targetPos, pos);

    float stopDist = 1.2f;

    if (len > stopDist)
    {
        e->Move(dir, dt);
        e->Face(dir);
    }

    //救出処理
    auto forward = e->GetForward();
    auto toTarget = MathHelper::Normalize(MathHelper::Subtract(targetPos, pos));
    float dot = MathHelper::Dot(forward, toTarget);
    float facingThreshold = 0.8f;

    if (len < 1.5f && dot > facingThreshold)
    {
        e->SetIsRescue(true);

        e->rescueTimer += dt;

        // 移動しながらでもOKにする
        //e->Move(dir, dt * 0.5f); // ゆっくり追いながら切る

        if (e->rescueTimer >= e->prepareTimeInterval && !e->isCutting)
        {
            e->isCutting = true;
            e->scissorsCutTimer = 0.0f;
        }
        if (e->isCutting)
        {
            if (e->scissorsCutTimer >= e->cutTimeInterval)
            {
                target->ReleasedTied();
                rescueCooldown = MathHelper::RandomRange(0.8f, 1.2f);   // クールタイムを設定
                CoreAudio::PlayOneShot(L"./Data/Sound/SE1/scissors_attack.wav");

                // 予約解除
                if (target->reservedBy == e)
                {
                    target->reservedBy = nullptr;
                }

                target = nullptr;
                e->rescueTimer = 0.0f;
                e->isCutting = false;
                e->scissorsCutTimer = 0.0f;
                e->SetIsRescue(false);
            }
        }
        return;
    }
    e->rescueTimer = 0.0f;
}

// 徘徊処理
void RescueBehavior::UpdateWander(EnemyBase* e, float dt)
{
    auto pos = e->GetPosition();

    float dist = MathHelper::Distance(pos, wanderTarget);

    // 近づいたら次の目的地
    if (dist < 1.0f)
    {
        wanderTarget =
        {
            MathHelper::RandomRange(e->rng, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX),
            0.0f,
            MathHelper::RandomRange(e->rng, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ)
        };
    }

    auto dir = MathHelper::Normalize(MathHelper::Subtract(wanderTarget, pos));

    e->Move(dir, dt);
    e->Face(dir);

    e->SetIsRescue(false);
    e->isCutting = false;
}

EnemyBase* RescueBehavior::FindTiedEnemy(const EnemyBase* self)
{
    auto scene = self->GetOwnerScene();
    auto enemies = scene->GetActorManager()->GetActorsOfType<EnemyBase>();

    EnemyBase* best = nullptr;
    float bestScore = FLT_MAX;

    for (auto enemy : enemies)
    {
        if (enemy.get() == self) continue;
        auto state = enemy->GetState();
        if (!(state == EnemyBase::YarnState::Tied || state == EnemyBase::YarnState::Tying)) continue;

        if (enemy->reservedBy != nullptr)
        {// 予約されてたらスキップ
            // 予約者が死んでたら解除
            if (enemy->reservedBy->IsDead())
            {
                enemy->reservedBy = nullptr;
            }
            else
            {
                continue;
            }
        }

        float dist = MathHelper::Distance(self->GetPosition(), enemy->GetPosition());

        float priority = (state == EnemyBase::YarnState::Tied) ? 0.0f : 5.0f;
        float score = dist + priority;


        if (score < bestScore)
        {
            bestScore = score;
            best = enemy.get();
        }
    }

    return best;
}