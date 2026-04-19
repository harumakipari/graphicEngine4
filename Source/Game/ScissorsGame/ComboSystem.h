#pragma once

class ComboSystem
{
public:
    void Update(float deltaTime)
    {
        if (comboCount <= 0) return;

        comboTimer -= deltaTime;

        if (comboTimer <= 0.0f)
        {
            Reset();
        }
    }

    void Reset()
    {
        comboCount = 0;
        comboTimer = 0.0f;
    }

    void AddHit()
    {
        comboCount++;
        comboTimer = comboResetTime;
    }

    int GetComboCount() const
    {
        return comboCount;
    }

    float GetMultiplier() const
    {
        if (comboCount <= 1) return 1.0f;
        if (comboCount == 2) return 1.3f;
        if (comboCount == 3) return 1.5f;
        if (comboCount == 4) return 1.7f;
        return 2.0f;
    }

private:
    int comboCount = 0;

    float comboTimer = 0.0f;
    float comboResetTime = 1.5f; // コンボをリセットするまでの秒数
};