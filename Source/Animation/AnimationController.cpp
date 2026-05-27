#include "pch.h"
#include "AnimationController.h"

#include <imgui.h>
#include <ranges>

#include "Core/Actor.h"

void AnimationController::OnUpdate(const float deltaTime)
{
    animationTime += deltaTime * animationRate;

    if (target_->model->animations.size() == 0)
    {// アニメーションがないモデルの場合
        return;
    }

    // アニメーション遷移の準備
    switch (transitionState)
    {
    case AnimationTransitionState::NotStarted:
        target_->model->Animate(this->animationClip, animationTime, animationNodes[0]);
        target_->model->Animate(this->animationNextClip, 0.0f, animationNodes[1]);
        transitionState = AnimationTransitionState::Inprogress;
        animationTime = 0.0f;
        blendFactor = 0.0f;
        break;
    case AnimationTransitionState::Inprogress:
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
    case AnimationTransitionState::Completed:
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

    // 描画に使うノードをブレンド結果にする
    target_->modelNodes = blendAnimationNodes;

    // --- ルートモーション処理 ---
    if (enableRootMotion)
    {
        InterleavedGltfModel::Node& node = blendAnimationNodes.at(rootNodeIndex);
        DirectX::XMFLOAT4X4 worldTransform = owner->GetWorldTransform();

        DirectX::XMFLOAT3 position = { node.globalTransform._41, node.globalTransform._42, node.globalTransform._43 }; // グローバル空間
        Logger::Log(U8("RootMotionのposition x:") + std::to_string(position.x) + U8("y:") + std::to_string(position.y) + U8("z:") + std::to_string(position.z));

        DirectX::XMFLOAT3 displacement = { position.x - previousPosition.x, position.y - previousPosition.y,  position.z - previousPosition.z }; // グローバル空間
        Logger::Log(U8("RootMotionのdisplacement x:") + std::to_string(displacement.x) + U8("y:") + std::to_string(displacement.y) + U8("z:") + std::to_string(displacement.z));
        DirectX::XMStoreFloat3(&displacement, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&displacement), DirectX::XMLoadFloat4x4(&worldTransform))); // ワールド空間

        DirectX::XMFLOAT3 translation = owner->GetPosition();

        translation.x += displacement.x;
        translation.y += displacement.y;
        translation.z += displacement.z;

        previousPosition = position;
        node.translation = zeroTranslation;

        target_->model->CumulateTransforms(blendAnimationNodes);

        owner->SetPosition(translation);
    }


}

void AnimationController::ResetRootMotion(int newClipIndex)
{
    animationClip = newClipIndex;
    animationTime = 0;

    target_->model->Animate(animationClip, 0, blendAnimationNodes);
    InterleavedGltfModel::Node& node = blendAnimationNodes.at(rootNodeIndex);
    previousPosition = { node.globalTransform._41, node.globalTransform._42, node.globalTransform._43 }; // グローバル空間
    zeroTranslation = node.translation;
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
    ImGui::SliderFloat("Rate", &animationRate, 0.0f, 3.0f);

    ImGui::Checkbox("enableRootMotion", &enableRootMotion);

    ImGui::Separator();

    for (const auto& name : animationImGUiOrder)
    {
        if (ImGui::Button(name.c_str()))
        {
            SetAnimationClip(name, isAnimationLoop, isBlendingAnimation, transitionTime);
        }
    }
#endif
}
