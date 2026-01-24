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

    // 進んだ拍数を取得してリセットする
    int ConsumeAdvancedBeatCount();

    //　ビート間の時間を設定する
    void SetBeatInterval(const double beatInterval) { this->beatInterval = beatInterval; }
private:
    // ビートを進める
    void AdvanceBeat();

private:
    double time = 0.0;
    float beatPhase = 0.0f;

    int beatIndex = -1;
    int barIndex = 0;
    int beatsPerBar = 4;
    int advancedBeatCount = 0;

    BeatInfo currentBeat;

    bool beatJustChanged = false;


    double gameTime = 0.0;
    double beatInterval = 1.5;

};