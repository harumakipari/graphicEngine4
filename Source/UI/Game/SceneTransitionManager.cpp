#include "pch.h"
#include "SceneTransitionManager.h"

#include "TransitionEffect.h"
#include "Engine/Scene/Scene.h"


void SceneTransitionManager::Initialize()
{
}


void SceneTransitionManager::RequestTransition(
    const std::string& nextScene,
    const SceneTransitionParam& param)
{
    if (state_ != State::Idle)
        return;

    nextScene_ = nextScene;

    // Effect¶¬
    transitionEffect = Scene::GetCurrentScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ScaleTransitionEffect>("SceneTransitionEffect");

    this->param = param;

    state_ = State::Playing;
}

void SceneTransitionManager::Update(float deltaTime)
{
    if (state_ == State::Playing)
    {
        if (transitionEffect && transitionEffect->IsFinished())
        {
            state_ = State::ChangingScene;

            Scene::_transition(nextScene_, param);

            state_ = State::Idle;
        }
    }
}