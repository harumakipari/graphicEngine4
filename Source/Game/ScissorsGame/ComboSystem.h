#pragma once

class ComboSystem
{
public:
    void Update(float deltaTime, bool isDashing)
    {
        if (comboCount <= 0) return;

        if (!isDashing)
        {
            comboTimer -= deltaTime;

            if (comboTimer <= 0.0f)
            {
                Reset();
            }
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
        //comboTimer = comboResetTime + comboCount * 0.1f;
    }

    int GetComboCount() const
    {
        return comboCount;
    }

    float GetMultiplier() const
    {
#if 1
        if (comboCount <= 1) return 1.0f;
        if (comboCount <= 3) return 1.2f;
        if (comboCount <= 5) return 1.5f;
        if (comboCount <= 7) return 1.7f;
        if (comboCount <= 9) return 2.0f;
        return 2.0f;
#else
        if (comboCount <= 1) return 1.0f;
        if (comboCount <= 3) return 1.2f;
        if (comboCount <= 5) return 1.5f;
        if (comboCount <= 7) return 2.0f;
        if (comboCount <= 9) return 3.0f;
        return 4.0f; 
#endif // 0

    }

private:
    int comboCount = 0;

    float comboTimer = 0.0f;
    float comboResetTime = 1.3f; // コンボをリセットするまでの秒数
};