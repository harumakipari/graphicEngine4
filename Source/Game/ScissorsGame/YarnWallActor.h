#pragma once
#include "Core/Actor.h"

enum class WallBehavior : uint8_t
{
    AppearOnce, // 最初隠れてて、時間で出る
    HideOnce    // 最初見えてて、時間で消える
};

class YarnWallActor :public Actor
{
    enum class WallState :uint8_t
    {
        Hidden,     // 地下
        Rising,     // 上昇中
        Visible,    // 出現中
        Lowering    // 下降中
    };

public:
    explicit YarnWallActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // AppearTimeを設定したあとに呼び出す関数
    void SetUp();

    void SetBehavior(WallBehavior newBehavior)
    {
        behavior = newBehavior;
    }

    // 何秒後に出現
    void SetAppearTime(float time) { appearTime = time; }

    // 何秒後に消える
    void SetHideTime(float time) { hideTime = time; }


    void SetTriggerWave(int wave)
    {
        triggerWave = wave;
    }

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent; // 描画コンポーネント
    std::shared_ptr<BoxComponent> redirectCollisionComponent;// 反射コンポーネント

    WallState state = WallState::Visible;
    WallBehavior behavior = WallBehavior::HideOnce;

    float elapsedTime = 0.0f;

    float appearTime = 0.0f;
    float hideTime = -1.0f;

    float moveSpeed = 6.0f;

    float hiddenY = -10.0f;
    float visibleY = 0.0f;

    int triggerWave = -1; // トリガーとなるwaveの数値
    bool triggered = false;
};

