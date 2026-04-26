#pragma once

class EnemyBase;

class EnemyBehavior
{
public:
    virtual void Enter(EnemyBase* e) {}
    virtual void Update(EnemyBase* e, float dt) = 0;
    virtual void Exit(EnemyBase* e) {}
};

class StaticBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override {}

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}
};

class LinearBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override {}

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}
};

class WaveHorizontalBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override {}

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}

private:
    // ”g‘Å‚¿ˆÚ“®‚Ìƒpƒ‰ƒ[ƒ^
    float waveTime = 0.0f;
    float waveAmplitude = 1.0f; // U‚ê•
    float waveFrequency = 3.0f; // ‘¬‚³
};

class WaveVerticalBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override {}

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}

private:
    // ”g‘Å‚¿ˆÚ“®‚Ìƒpƒ‰ƒ[ƒ^
    float waveTime = 0.0f;
    float waveAmplitude = 1.0f; // U‚ê•
    float waveFrequency = 3.0f; // ‘¬‚³
};

class ChaseBehavior : public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override {}

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}

private:
    float avoidDist = 1.5f; // “G‚ÌL‚ª‚è
    float separationWeight = 0.3f; // ‚Ç‚ê‚­‚ç‚¢‚Î‚ç‚¯‚é‚©

};

class RescueBehavior :public EnemyBehavior
{
public:
    void Enter(EnemyBase* e) override;

    void Update(EnemyBase* e, float dt) override;

    void Exit(EnemyBase* e) override {}

private:
    // ‹Ê~‚ß‚³‚ê‚Ä‚¢‚é“G‚ğ’T‚·
    EnemyBase* FindTiedEnemy(const EnemyBase* self);

    EnemyBase* target = nullptr;

};
