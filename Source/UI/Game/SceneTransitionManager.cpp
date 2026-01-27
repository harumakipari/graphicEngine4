#include "pch.h"
#include "SceneTransitionManager.h"

#include "TransitionEffect.h"
#include "Engine/Scene/Scene.h"


void SceneTransitionManager::Initialize()
{
    transitionEffect = std::make_shared<ScaleTransitionEffect>();
    transitionEffect->Initialize();
}

const char* ToString(SceneTransitionManager::State s)
{
    switch (s)
    {
    case SceneTransitionManager::State::Idle: return "Idle";
    case SceneTransitionManager::State::Closing: return "Closing";
    case SceneTransitionManager::State::ChangingScene: return "ChangingScene";
    case SceneTransitionManager::State::Opening: return "Opening";
    }
    return "Unknown";
}

void SceneTransitionManager::RequestTransition(
    const std::string& nextScene,
    const SceneTransitionParam& param)
{
    if (state_ != State::Idle)
    {
        Logger::Warning((ToString(state_)));
        return;
    }
    nextScene_ = nextScene;

    // Effect生成
    transitionEffect->OnSceneChanged();
    transitionEffect->Start(TransitionDirection::Close);

    this->param = param;
    state_ = State::Closing;
}

void SceneTransitionManager::Update(float deltaTime)
{
    if (!transitionEffect)
        return;

    transitionEffect->Update(deltaTime);

    switch (state_)
    {
    case State::Closing:
        if (transitionEffect->IsFinished())
        {
            state_ = State::ChangingScene;
            Scene::_transition(nextScene_, param);
            Logger::Log(U8("Closion を通った"));
        }
        break;

    case State::ChangingScene:
        // 新シーン生成後に呼ばれる想定
        break;

    case State::Opening:
        if (transitionEffect->IsFinished())
        {
            //transitionEffect.reset();
            state_ = State::Idle;
            Logger::Log(U8("Opening を通った"));

            if (onOpeningFinished)
            {
                onOpeningFinished();
                onOpeningFinished = nullptr;
            }
        }
        break;
    }
}

void SceneTransitionManager::SetOnOpeningFinished(const std::function<void()>& callback)
{
    onOpeningFinished = callback;
}


void SceneTransitionManager::NotifySceneChanged()
{
    if (state_ == State::ChangingScene)
    {
        param.clear();
        transitionEffect->OnSceneChanged();
        transitionEffect->Start(TransitionDirection::Open);
        state_ = State::Opening;
    }
}
