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
        InterleavedGltfModel::Node& rootNode = blendAnimationNodes.at(rootNodeIndex);

        //DirectX::XMFLOAT4X4 worldTransform = owner->GetWorldTransform();

        // アニメーション上のグローバル移動量
        DirectX::XMFLOAT3 currentGlobalPos = { rootNode.globalTransform._41, rootNode.globalTransform._42, rootNode.globalTransform._43 };
        DirectX::XMFLOAT3 delta = { currentGlobalPos.x - previousPosition.x,
                                    currentGlobalPos.y - previousPosition.y,
                                    currentGlobalPos.z - previousPosition.z };

        // ワールド空間に変換
        DirectX::XMFLOAT4X4 worldTransform = owner->GetWorldTransform();
        DirectX::XMStoreFloat3(&delta, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&delta), DirectX::XMLoadFloat4x4(&worldTransform)));

        // 移動量のスケーリング（固定速度や倍率をここで調整可能）
        float speedScale = 1.0f; // 1.0 = そのまま、0.5 = 半分、2.0 = 2倍など
        delta.x *= speedScale;
        delta.y *= speedScale;
        delta.z *= speedScale;

        // owner のワールド位置に加算（ルートモーションを反映）
        DirectX::XMFLOAT3 translation = owner->GetPosition();
        translation.x += delta.x;
        translation.y += delta.y;
        translation.z += delta.z;
        owner->SetPosition(translation);

        // previousPosition を更新（アニメーション上の位置を追う）
        previousPosition = currentGlobalPos;

        // ルートノードはローカル初期値に戻す
        rootNode.translation = zeroTranslation;

        // 子ノードのグローバル変換を再帰的に更新
        target_->model->CumulateTransforms(blendAnimationNodes);
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
    ImGui::SliderFloat("Rate", &animationRate, 0.1f, 3.0f);

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
