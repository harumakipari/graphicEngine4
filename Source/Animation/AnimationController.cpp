#include "pch.h"
#include "AnimationController.h"

#include <imgui.h>

void AnimationController::OnUpdate(const float deltaTime)
{
    animationTime += deltaTime * animationRate;

    if (target_->model->animations.size() == 0)
    {// アニメーションがないモデルの場合
        return;
    }

    if (isBlendingAnimation && transitionTime > 0.0f)
    {

    }

    switch (transitionState)
    {
    case AnimationController::AnimationTransitionState::NotStarted:
        target_->model->Animate(this->animationClip, animationTime, animationNodes[0]);
        target_->model->Animate(this->animationNextClip, 0.0f, animationNodes[1]);
        transitionState = AnimationTransitionState::Inprogress;
        animationTime = 0.0f;
        blendFactor = 0.0f;
        break;
    case AnimationController::AnimationTransitionState::Inprogress:
        if (transitionTime > 0.0f)
        {
            blendFactor = animationTime / transitionTime;     //ゼロ除算を防ぐため
        }
        else
        {
            blendFactor = 1.0f;
        }
        target_->model->BlendAnimations(animationNodes[0], animationNodes[1], blendFactor, blendAnimationNodes);
        if (blendFactor >= 1.0f)
        {
            // 遷移終了
            transitionState = AnimationTransitionState::Completed;
            animationTime = 0.0f;
            // 現在のアニメーションクリップを次のアニメーションクリップに変更する
            this->animationClip = this->animationNextClip;
            //isBlendingAnimation = false;
        }
        break;
    case AnimationController::AnimationTransitionState::Completed:
        // 終わったら通常時に戻す
        //isBlendingAnimation = false;
        if (target_->model->animations.at(animationClip).duration < animationTime)
        {
            if (isAnimationLoop)
            {//アニメーションをループするとき
                if (requestStopLoop)
                {
                    isAnimationLoop = false;    // ループしないモードにする
                    animationTime = 0.0f;
                    requestStopLoop = false;
                }
                else
                {
                    animationTime = 0;
                }
            }
            else
            {
                isAnimationFinished = true;
            }
        }
        target_->model->Animate(animationClip, animationTime, blendAnimationNodes);
        break;
    default:
        break;
    }
    // 描画に使うノードをブレンドのノードにする
    //target_->model->nodes = blendAnimationNodes;
    target_->modelNodes = blendAnimationNodes;
}

void AnimationController::DrawImGui()
{
#ifdef USE_IMGUI
    if (!ImGui::CollapsingHeader("Animation Debug"))
        return;

    ImGui::Text("Current: %s", currentAnimationName.c_str());
    ImGui::Text("Playing: %s", isAnimationFinished ? "No" : "Yes");

    ImGui::Checkbox("Loop", &isAnimationLoop);
    ImGui::Checkbox("Blend", &isBlendingAnimation);
    ImGui::SliderFloat("Blend Time", &transitionTime, 0.0f, 1.0f);
    ImGui::SliderFloat("Rate", &animationRate, 0.1f, 3.0f);

    ImGui::Separator();

    for (const auto& [name, index] : animationNameToIndex_)
    {
        if (ImGui::Button(name.c_str()))
        {
            SetAnimationClip(name, isAnimationLoop, isBlendingAnimation, transitionTime);
        }
    }
#endif
}
