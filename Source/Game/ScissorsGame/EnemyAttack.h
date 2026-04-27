#pragma once

class EnemyBase;

class EnemyAttack
{
public:
    virtual void Update(EnemyBase* e, float dt) {}
};


class NeedleAttack :public EnemyAttack
{
    enum class AttackState :uint8_t
    {
        Idle,
        Charge,
        Fire
    };
public:
    void Update(EnemyBase* e, float dt) override;

private:
    // —­‚ß‚éik‚Şj
    void UpdateCharge(EnemyBase* e, float dt);

    // ”­Ë
    void UpdateFire(EnemyBase* e, float dt);

    // ƒ‹[ƒv‚Å‰~ó‚É•ú‚Â
    void FireRadial(EnemyBase* e);

    // œpœjXV
    void UpdateWander(EnemyBase* e, float dt);

    AttackState state = AttackState::Idle;

    float stateTimer = 2.0f;   // Å‰‚Ì‘Ò‹@ŠÔ

    // Šeó‘Ô‚ÌŠÔ
    float interval = 3.0f;   // IdleŠÔ
    float chargeTime = 1.0f;   // —­‚ß
    float fireTime = 0.2f;   // ”­Ë

    // œpœj—p
    DirectX::XMFLOAT3 wanderTarget = {};

    bool useEight = false;// ‚S–{‚W–{‚ğŒğŒİ‚É‚·‚é
    DirectX::XMFLOAT3 basePos = { 0.0f,0.0f,0.0f };
};