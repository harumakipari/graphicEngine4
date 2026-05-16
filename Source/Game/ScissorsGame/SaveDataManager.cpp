#include "pch.h"
#include "SaveDataManager.h"

#include <fstream>
#include <json.hpp>


SaveDataManager& SaveDataManager::Instance()
{
    static SaveDataManager instance;
    return instance;
}

// 初期化
void SaveDataManager::Initialize()
{
    Load();
}

// クリア保存
void SaveDataManager::SetStageClear(STAGE_NAME stage, bool clear)
{
    saveData.clearedStages[stage] = clear;

    Save();
}

// クリア確認
bool SaveDataManager::IsStageCleared(STAGE_NAME stage) const
{
    auto it = saveData.clearedStages.find(stage);

    if (it == saveData.clearedStages.end())
    {
        return false;
    }

    return it->second;
}

// デバッグ用
void SaveDataManager::Reset()
{
    // クリア情報を全部削除
    saveData.clearedStages.clear();

    // セーブファイル更新
    Save();
}

// 全てのステージを開放する
void SaveDataManager::UnlockAllStage()
{
    saveData.clearedStages[STAGE_NAME::TUTORIAL] = true;
    saveData.clearedStages[STAGE_NAME::FIRST] = true;
    saveData.clearedStages[STAGE_NAME::BOBBIN_FIRST] = true;
    saveData.clearedStages[STAGE_NAME::REFLECT_WALL] = true;
    saveData.clearedStages[STAGE_NAME::BOBBIN_SECOND] = true;
    saveData.clearedStages[STAGE_NAME::DIFFICULT] = true;
    saveData.clearedStages[STAGE_NAME::BOSS] = true;

    Save();
}

// ステージを開放するかどうか
bool SaveDataManager::IsStageUnlocked(const STAGE_NAME stage) const
{
    switch (stage)
    {
    case STAGE_NAME::TUTORIAL:
        return true;
    case STAGE_NAME::FIRST:
    case STAGE_NAME::BOBBIN_FIRST:
        return IsStageCleared(STAGE_NAME::TUTORIAL);
    case STAGE_NAME::REFLECT_WALL:
    case STAGE_NAME::BOBBIN_SECOND:
    case STAGE_NAME::DIFFICULT:
    case STAGE_NAME::BOSS:
        return IsStageCleared(STAGE_NAME::BOBBIN_FIRST);
    default:
        return false;
    }
}

// 保存
void SaveDataManager::Save()
{
    nlohmann::json json;

    for (auto& [stage, clear]
        : saveData.clearedStages)
    {
        json["clearedStages"]
            [std::to_string((int)stage)] = clear;
    }

    std::ofstream file(savePath);

    if (file.is_open())
    {
        file << json.dump(4);
    }
}

// ロード
void SaveDataManager::Load()
{
    saveData.clearedStages.clear();

    std::ifstream file(savePath);

    if (!file.is_open())
    {
        return;
    }

    nlohmann::json json;
    file >> json;

    if (json.contains("clearedStages"))
    {
        for (auto& [key, value]
            : json["clearedStages"].items())
        {
            STAGE_NAME stage =
                static_cast<STAGE_NAME>(std::stoi(key));

            saveData.clearedStages[stage]
                = value.get<bool>();
        }
    }
}


