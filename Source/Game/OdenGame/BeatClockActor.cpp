#include "pch.h"
#include "BeatClockActor.h"




void BeatClockActor::Update(float deltaTime)
{
    time += deltaTime;

    float beatTime = 60.0f / bpm;
    while (time >= beatTime)
    {
        time -= beatTime;
        AdvanceBeat();
    }

    beatPhase = time / beatTime; // 0.0~1.0
}

void BeatClockActor::AdvanceBeat()
{
    beatIndex++;
    if (beatIndex % beatsPerBar == 0)
    {
        barIndex++;
    }

    currentBeat =
    {
        beatIndex,
        barIndex,
        (beatIndex % beatsPerBar) == 0 // ã≠îè
    };

    beatJustChanged = true;
}