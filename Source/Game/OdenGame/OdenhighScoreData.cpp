#include "pch.h"
#include "OdenHighScoreData.h"

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;


namespace
{
    const char* SAVE_DIR = "./Data/SaveData";
    const char* SAVE_FILE = "./Data/SaveData/OdenHighScore.json";
}

void OdenHighScoreData::Save()
{
    // ディレクトリがなければ作る
    std::filesystem::create_directories(SAVE_DIR);

    json j;

    j["Easy"] = highScores[static_cast<size_t>(GameDifficulty::Easy)];
    j["Normal"] = highScores[static_cast<size_t>(GameDifficulty::Normal)];
    j["Hard"] = highScores[static_cast<size_t>(GameDifficulty::Hard)];

    std::ofstream ofs(SAVE_FILE);
    if (!ofs.is_open())
    {
        Logger::Log("Failed to save high score json");
        return;
    }

    ofs << j.dump(4); // インデント付き
    ofs.close();

    Logger::Log("HighScore Saved");
}

void OdenHighScoreData::Load()
{
    std::ifstream ifs(SAVE_FILE);
    if (!ifs.is_open())
    {
        Logger::Log("HighScore file not found. Use default.");
        highScores.fill(0);
        return;
    }

    json j;
    ifs >> j;
    ifs.close();

    highScores[static_cast<size_t>(GameDifficulty::Easy)] = j.value("Easy", 0);
    highScores[static_cast<size_t>(GameDifficulty::Normal)] = j.value("Normal", 0);
    highScores[static_cast<size_t>(GameDifficulty::Hard)] = j.value("Hard", 0);

    Logger::Log("HighScore Loaded");
}