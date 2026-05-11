#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"

// タイトルステージモデルアクター
class TitleBookActor :public Actor
{
public:
    explicit TitleBookActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // 本を開く
    void Play(float interval);

    // 本を閉じる
    void PlayReverse(float interval);

private:
    std::shared_ptr<SkeletalMeshComponent> bookLeftModel;
    std::shared_ptr<SkeletalMeshComponent> bookRightModel;

    std::shared_ptr<SkeletalMeshComponent> patchTutorialModel;

    std::unique_ptr<EasingRunner> easingRunner;

    float startEuler = 0.0f;
    float endEuler = 180.0f;

    float angle = 0.0f;

};

