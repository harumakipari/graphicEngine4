#pragma once
#include "TransitionEffect.h"

class SceneTransitionManager
{
public:
    using SceneTransitionParam = std::unordered_map<std::string, std::string>;

    static SceneTransitionManager& Instance()
    {
        static SceneTransitionManager instance;
        return instance;
    }

    void Initialize();

    void RequestTransition(
        const std::string& nextScene,
        const SceneTransitionParam& param = {}
    );

    void Update(float deltaTime);
    void Draw(){}

private:
    enum class State :uint8_t
    {
        Idle,
        Playing,
        ChangingScene
    };

    State state_ = State::Idle;

    std::string nextScene_;
    SceneTransitionParam param;

    std::shared_ptr<ScaleTransitionEffect> transitionEffect;
};
