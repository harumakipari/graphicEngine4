#include "pch.h"
#include "BeatClockActor.h"

#include <chrono>


void BeatClockActor::Update(float deltaTime)
{
    gameTime += deltaTime;

    int currentBeatIndex = static_cast<int>(gameTime / beatInterval);

    if (currentBeatIndex != beatIndex)
    {
        beatIndex = currentBeatIndex;
        AdvanceBeat();
    }
    beatPhase = gameTime / beatInterval; // 0.0~1.0


    //time += deltaTime;

    //double beatTime = 1.5;
    //while (time >= beatTime)
    //{
    //    time -= beatTime;
    //    AdvanceBeat();
    //}

    // beatPhase = time / beatTime; // 0.0~1.0
}


// 進んだ拍数を取得してリセットする
int BeatClockActor::ConsumeAdvancedBeatCount()
{
    int count = advancedBeatCount;
    advancedBeatCount = 0;
    return count;
}

void BeatClockActor::AdvanceBeat()
{

    double idealTime = beatIndex * beatInterval;

    Logger::Log("Beat at: " + std::to_string(idealTime));


    advancedBeatCount++;

    if (beatIndex % beatsPerBar == 0)
    {
        barIndex++;
    }

    currentBeat =
    {
        beatIndex,
        barIndex,
        (beatIndex % beatsPerBar) == 0 // 強拍
    };

    beatJustChanged = true;
}