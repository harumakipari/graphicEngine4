#include "pch.h"
#include "ScoreHistoryManager.h"
#include <json.hpp>
#include <fstream>
#include <magic_enum.hpp>

// スコアを提出する
void ScoreHistoryManager::Submit(STAGE_NAME stage, int score)
{
    auto& list = rankings[stage];

    // 同じスコアが既に存在するなら追加しない
    auto it = std::find_if(
        list.begin(),
        list.end(),
        [score](const Entry& entry)
        {
            return entry.score == score;
        });

    if (it != list.end())
    {
        return;
    }



    list.push_back({ score });

    std::sort(list.begin(), list.end(),
        [](const Entry& a, const Entry& b)
        {
            return a.score > b.score;
        });

    if (list.size() > 5)
    {
        list.resize(5);
    }

    Save();
}

// Top5のスコアを取得する
const std::vector<ScoreHistoryManager::Entry>& ScoreHistoryManager::GetTop5(STAGE_NAME stage)
{
    static std::vector<Entry> empty;

    auto it = rankings.find(stage);

    if (it == rankings.end())
    {
        return empty;
    }

    return it->second;
}

// ハイスコアのみを取得する
int ScoreHistoryManager::GetHighScore(STAGE_NAME stage)
{
    auto it = rankings.find(stage);

    if (it == rankings.end() || it->second.empty())
    {
        return 0;
    }

    return it->second.front().score;
}

// データをファイルからロードする
void ScoreHistoryManager::Load()
{
    rankings.clear();

    std::ifstream file("./Data/Saves/score.json");

    if (!file.is_open())
    {
        return;
    }

    using json = nlohmann::json;

    json root;
    file >> root;

    for (auto& [stageName, array] : root.items())
    {
        STAGE_NAME stage = StringToStageName(stageName);

        for (auto& item : array)
        {
            Entry entry;
            entry.score = item.value("score", 0);

            rankings[stage].push_back(entry);
        }
    }
}

// STAGE_NAMEを変換する関数
STAGE_NAME ScoreHistoryManager::StringToStageName(const std::string& name)
{
    if (name == "TUTORIAL")
        return STAGE_NAME::TUTORIAL;

    if (name == "FIRST")
        return STAGE_NAME::FIRST;

    if (name == "BOBBIN_FIRST")
        return STAGE_NAME::BOBBIN_FIRST;

    if (name == "REFLECT_WALL")
        return STAGE_NAME::REFLECT_WALL;

    if (name == "BOBBIN_SECOND")
        return STAGE_NAME::BOBBIN_SECOND;

    if (name == "DIFFICULT")
        return STAGE_NAME::DIFFICULT;

    if (name == "BOSS")
        return STAGE_NAME::BOSS;

    return STAGE_NAME::FIRST;
}

// 新記録かどうか
bool ScoreHistoryManager::IsNewRecord(STAGE_NAME stage, int score)
{
    auto it = rankings.find(stage);

    // まだ記録が無いなら新記録
    if (it == rankings.end() || it->second.empty())
    {
        return true;
    }

    // 現在の1位より高ければ新記録
    return score >= it->second.front().score;
}

// ランキング
int ScoreHistoryManager::GetRanking(STAGE_NAME stage, int score)
{

    auto it = rankings.find(stage);

    if (it == rankings.end())
    {
        return -1;
    }

    const auto& list = it->second;

    for (int i = 0; i < list.size(); i++)
    {
        if (list[i].score == score)
        {
            return i; // 0-based
        }
    }

    return -1;

}

// データをjsonに保存する
void ScoreHistoryManager::Save()
{
    using json = nlohmann::json;

    json root;

    for (const auto& [stage, scores] : rankings)
    {
        json array = json::array();

        for (const auto& entry : scores)
        {
            array.push_back({
                { "score", entry.score }
                });
        }

        root[std::string(
            magic_enum::enum_name(stage))] = array;
    }

    std::ofstream file("./Data/Saves/score.json");

    if (file.is_open())
    {
        file << root.dump(4);
    }
}
