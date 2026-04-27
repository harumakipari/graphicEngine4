#pragma once

class EnemyBase;

class EnemyAttack
{
public:
    virtual void Update(EnemyBase* e, float dt) {}
};


class NeedleAttack:public EnemyAttack
{
public:
    void Update(EnemyBase* e, float dt) override;

private:
    // ”­ŽË‚·‚é
    void Fire(EnemyBase* e);

    float cooldownTimer = 0.0f;  
    float interval = 2.0f;  // ”­ŽË‚·‚éŠÔŠu
};