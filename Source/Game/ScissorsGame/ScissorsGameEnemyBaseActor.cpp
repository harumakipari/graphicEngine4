#include "pch.h"
#include "ScissorsGameEnemyBaseActor.h"

void ScissorsGameEnemyBase::Initialize(const Transform& transform)
{
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
        if (onDeath)
        {// WaveManagerにenemyCountを減らすように通知する
            onDeath();
        }
        MarkPendingKill();
        if (starEffectComponent)
        {
            starEffectComponent->Play();
        }
        return true;
    }
    return false;

}

