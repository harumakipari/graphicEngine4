#pragma once
#include "OdenData/OdenDataStruct.h"

enum class OdenRank :uint8_t
{
    D,
    C,
    B,
    A,
    S,
    Count
};

struct RankThreshold
{
    OdenRank rank;
    int minScore;   // この点以上でこのランク
};

struct RankResult
{
    OdenRank current;
    int nextRankScore;   // 次ランクに必要なスコア
    bool isMaxRank;
};


inline std::vector<RankThreshold> GetRankTable(GameDifficulty diff)
{
    const std::vector<RankThreshold> hardRankTable =
    {
        { OdenRank::S, 4000 },
        { OdenRank::A, 3000 },
        { OdenRank::B, 2000 },
        { OdenRank::C,  1000 },
        { OdenRank::D,     0 }
    };

    const std::vector<RankThreshold> normalRankTable =
    {
        { OdenRank::S, 4000 },
        { OdenRank::A, 3000 },
        { OdenRank::B, 2000 },
        { OdenRank::C,  1000 },
        { OdenRank::D,     0 }
    };

    const std::vector<RankThreshold> easyRankTable =
    {
        { OdenRank::S, 4000 },
        { OdenRank::A, 3000 },
        { OdenRank::B, 2000 },
        { OdenRank::C,  1000 },
        { OdenRank::D,     0 }
    };

    switch (diff)
    {
    case GameDifficulty::Easy:
        return easyRankTable;
    case GameDifficulty::Normal:
        return normalRankTable;
    case GameDifficulty::Hard:
        return hardRankTable;
    case GameDifficulty::Count:
        break;
    }
    return normalRankTable;
}

inline RankResult EvaluateRank(GameDifficulty diff, int score)
{
    const auto& table = GetRankTable(diff);

    for (size_t i = 0; i < table.size(); ++i)
    {
        if (score >= table[i].minScore)
        {
            RankResult result;
            result.current = table[i].rank;

            if (i == 0)
            {
                result.isMaxRank = true;
                result.nextRankScore = 0;
            }
            else
            {
                result.isMaxRank = false;
                result.nextRankScore = table[i - 1].minScore;
            }
            return result;
        }
    }

    return { OdenRank::D, table.back().minScore, false };
}
