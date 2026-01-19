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

    static inline std::vector<OdenSubmitLog> submitLogs;
    static inline std::array<int, static_cast<size_t>(EOdenType::Count)> ingredientCount{};

    static void Reset()
    {
        totalScore = 0.0f;
        satisfaction = 0.0f;
        submitLogs.clear();
        ingredientCount = {};
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
