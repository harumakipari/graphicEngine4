#pragma once

#include "Core/Actor.h"

class TutorialManager;

class TutorialActor : public Actor
{
public:
    TutorialActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~TutorialActor() = default;
    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;
    // チュートリアルマネージャーを取得
    class TutorialManager* GetTutorialManager() const { return tutorialManager.get(); }
private:
    std::unique_ptr<TutorialManager> tutorialManager;
};