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

    // ƒV[ƒ“‚ªØ‚è‘Ö‚í‚Á‚½‚ÉŒÄ‚Î‚ê‚é
    void NotifySceneChanged();

    void Draw(){}

    enum class State :uint8_t
    {
        Idle,
        Closing,
        ChangingScene,
        Opening
    };

private:

    State state_ = State::ChangingScene;

    std::string nextScene_;
    SceneTransitionParam param;

    std::shared_ptr<ScaleTransitionEffect> transitionEffect;
};
