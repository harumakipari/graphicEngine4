#pragma once
#include "StageData.h"

class ScoreHistoryManager
{
public:
    struct Entry
    {
        int score = 0;
    };

public:
    // スコアを提出する
    static void Submit(STAGE_NAME stage, int score);

    // Top5のスコアを取得する
    static const std::vector<Entry>& GetTop5(STAGE_NAME stage);

    // ハイスコアのみを取得する
    static int GetHighScore(STAGE_NAME stage);

    // データをファイルからロードする
    static void Load();

    // 新記録かどうか
    static bool IsNewRecord(STAGE_NAME stage, int score);
private:
    // データをjsonに保存する
    static void Save();

    // STAGE_NAMEを変換する関数
    static STAGE_NAME StringToStageName(const std::string& name);

private:
    static inline std::unordered_map<STAGE_NAME, std::vector<Entry>> rankings;
};
