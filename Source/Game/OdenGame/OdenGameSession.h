#pragma once

class OdenGameSession
{
public:
    static OdenGameSession& Instance()
    {
        static OdenGameSession instance;
        return instance;
    }

    // ===== リザルト用データ =====
    static inline float totalScore = 0.0f;
    static inline float satisfaction = 0.0f;


    // ===== ボーナス用 =====
    static inline int maxCombo = 0;              // 最大連続正解
    static inline int missCount = 0;             // 注文ミス回数
    static inline int feverSubmitCount = 0;      // Fever中に出した具材数


    static inline std::vector<OdenSubmitLog> submitLogs;
    static inline std::array<int, static_cast<size_t>(EOdenType::Count)> ingredientCount{};

    static void Reset()
    {
        totalScore = 0.0f;
        satisfaction = 0.0f;
        submitLogs.clear();
        ingredientCount = {};
        maxCombo = 0;
        missCount = 0;
        feverSubmitCount = 0;
    }

    static void SetDifficulty(GameDifficulty diff)
    {
        gameDifficulty = diff;
    }

    static GameDifficulty GetDifficulty() 
    {
        return gameDifficulty;
    }

private:

    static inline GameDifficulty gameDifficulty = GameDifficulty::Normal;
};
