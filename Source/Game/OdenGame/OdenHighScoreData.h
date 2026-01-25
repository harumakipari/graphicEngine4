#pragma once

#include "OdenData/OdenDataStruct.h"


class OdenHighScoreData
{
public:
    static OdenHighScoreData& Instance()
    {
        static OdenHighScoreData instance;
        return instance;
    }

    int GetHighScore(GameDifficulty diff) const
    {
        return highScores[static_cast<size_t>(diff)];
    }

    bool TryUpdateHighScore(GameDifficulty diff, int score)
    {
        size_t index = static_cast<size_t>(diff);
        if (score > highScores[index])
        {
            highScores[index] = score;
            Save();
            return true;
        }
        return false;
    }

    void Save();
    void Load();

private:
    std::array<int, static_cast<size_t>(GameDifficulty::Count)> highScores{};

};
