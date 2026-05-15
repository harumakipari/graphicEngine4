#pragma once
#include "Core/Actor.h"


class HighScoreMedalActor :public Actor
{
public:
    explicit HighScoreMedalActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // I—¹‚Ìˆ—
    void Finalize() override;

    
    void Play();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;

};


