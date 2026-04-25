#pragma once

class EnemyBase;

class EnemyBehavior
{
public:
    virtual void Enter(EnemyBase* e) {}
    virtual void Update(EnemyBase* e, float dt) = 0;
    virtual void Exit(EnemyBase* e) {}
};

class IdleEnemyBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override
    {
    }

    void Update(EnemyBase* e, float dt) override
    {
    }

    void Exit(EnemyBase* e) override
    {
    }
};


#if 0
class ChaseBehavior : public EnemyBehavior
{
public:
    void Update(EnemyBase* e, float dt) override
    {
        auto player = e->GetPlayer();
        if (!player) return;

        auto pos = e->GetPosition();
        auto target = player->GetPosition();

        DirectX::XMFLOAT3 dir =
        {
            target.x - pos.x,
            0,
            target.z - pos.z
        };

        float len = sqrt(dir.x * dir.x + dir.z * dir.z);
        if (len < 0.001f) return;

        dir.x /= len;
        dir.z /= len;

        e->Move(dir, dt);
        e->Face(dir);
    }
};
#endif // 0
