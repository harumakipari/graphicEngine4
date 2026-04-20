#include "pch.h"
#include "ScissorsGameEnemyBaseActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void ScissorsGameEnemyBase::Initialize(const Transform& transform)
{
}

void ScissorsGameEnemyBase::Update(float deltaTime)
{
    XMFLOAT3 position = GetPosition();
    // 移動
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    SetPosition(position);

    // 減速（超重要）
    velocity.x -= velocity.x * friction * deltaTime;
    velocity.z -= velocity.z * friction * deltaTime;

    if (isDead)
    {
        velocity.y -= 9.8f * deltaTime;
        deathTimer += deltaTime;

        // 0.8秒後に消す
        if (deathTimer > 0.8f)
        {
            MarkPendingKill();
        }
    }

}

void ScissorsGameEnemyBase::MoveLinear(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ステージ端で反転
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;

    if (pos.x < stageMinX || pos.x > stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
    }

    if (pos.z < stageMinZ || pos.z > stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);
    }

    SetPosition(pos);
}

bool ScissorsGameEnemyBase::OnHitByDash(ScissorsPlayer1* player, int dashDamage)
{
    XMFLOAT3 playerPos = player->GetPosition();
    XMFLOAT3 enemyPos = GetPosition();

    XMFLOAT3 dir = MathHelper::Normalize(MathHelper::Subtract(enemyPos, playerPos));

    // 吹っ飛ばす
    ApplyKnockBack(dir);

    int prevHp = hp;
    // ダッシュで当たったときの処理
    TakeDamage(dashDamage);
    // コントローラーを振動させる
    InputSystem::SetVibration(0.8f, 0.15f);

    // 倒したかどうかを返す
    return (hp <= 0 && prevHp > 0);
}

// ダメージを与える　死亡したかどうかを取得する関数
bool ScissorsGameEnemyBase::TakeDamage(int damage)
{
    if (hp <= 0) return true; // すでに倒れている場合は無視

    hp -= damage;
    Logger::Log(U8("敵にダメージ：") + std::to_string(damage));
    if (hp <= 0)
    {
        isDead = true;

        if (onDeath)
        {// WaveManagerにenemyCountを減らすように通知する
            onDeath();
        }

        // エフェクトを発生させる
        SpawnHitEffect();

        //MarkPendingKill();


        if (starEffectComponent)
        {
            //starEffectComponent->Play();
        }
        return true;
    }
    return false;

}

// ヒットエフェクトを生成する
void ScissorsGameEnemyBase::SpawnHitEffect()
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    // ワールド→スクリーン変換（必要なら）
    XMFLOAT3 pos = GetPosition();
    XMFLOAT2 screenPos = WorldToUI(pos);

    // リング（少し遅らせると良い）
    auto ring = std::make_shared<UIRingEffect>("./Data/Textures/ScissorsUI/ring.png");
    ring->SetWorldPosition(screenPos);
    ring->SetSize({ 100,100 });
    uiManager->Add(ring);

    // 星
    for (int i = 0; i < 8; i++)
    {
        auto star = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png", screenPos);
        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }
}

void ScissorsGameEnemyBase::ApplyKnockBack(DirectX::XMFLOAT3 dir)
{
    float horizontalPower = 8.0f;
    float verticalPower = 4.0f; // ← 上は弱め

    velocity.x = dir.x * horizontalPower;
    velocity.z = dir.z * horizontalPower;
    velocity.y = verticalPower;
}
