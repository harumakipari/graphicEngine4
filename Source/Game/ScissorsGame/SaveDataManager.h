#pragma once
#include "StageData.h"


struct SaveData
{
    std::unordered_map<STAGE_NAME, bool> clearedStages;
};

class SaveDataManager
{
public:
    static SaveDataManager& Instance();

public:
    // 初期化
    void Initialize();

    // セーブ
    void Save();

    // ロード
    void Load();

    // ステージクリア設定
    void SetStageClear(STAGE_NAME stage, bool clear);

    // クリア済みか
    bool IsStageCleared(STAGE_NAME stage) const;

    // デバッグ用
    void Reset();

    // 全てのステージを開放する
    void UnlockAllStage();

    // ステージを開放するかどうか
    bool IsStageUnlocked(STAGE_NAME stage) const;

private:
    SaveDataManager() = default;

private:
    SaveData saveData;

    std::string savePath = "./Data/Saves/save.json";
};
