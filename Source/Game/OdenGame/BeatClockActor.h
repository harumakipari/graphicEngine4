#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"



class BeatClockActor :public Actor
{
public:
    struct BeatInfo
    {
        int beatIndex;     // 拍番号
        int barIndex;      // 小節
        bool isStrong;     // 強拍
    };

public:
    explicit BeatClockActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override {}

    void Update(float deltaTime)override;

    float GetBeatPhase() const { return beatPhase; }

    const BeatInfo& GetCurrentBeat() const { return currentBeat; }

    // 拍が変わったかどうか
    bool ConsumeBeatJustChanged()
    {
        if (beatJustChanged)
        {
            beatJustChanged = false;
            return true;
        }
        return false;
    }

    // BPMを設定する
    void SetBpm(const float bpm)
    {
        this->bpm = bpm;
    }
private:
    // ビートを進める
    void AdvanceBeat();

private:
    float bpm = 105.0f;
    float time = 0.0f;
    float beatPhase = 0.0f;

    int beatIndex = 0;
    int barIndex = 0;
    int beatsPerBar = 4;

    BeatInfo currentBeat;

    bool beatJustChanged = false;
};