#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

// C++ 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクトの他のヘッダ
#include "Components/Render/MeshComponent.h"
#include "Graphics/Resource/InterleavedGltfModel.h"

// アニメーションのコントローラー  
class AnimationController
{
public:
    AnimationController(SkeletalMeshComponent* target) :target_(target)
    {
        // アニメーションブレンドに使用するノード
        animationNodes[0] = target_->model->GetNodes();
        animationNodes[1] = target_->model->GetNodes();

        blendAnimationNodes = target_->model->GetNodes();
    }

    void AddAnimation(const std::string& animationName, const size_t animationClip)
    {
        animationNameToIndex_[animationName] = animationClip;
    }

    // アニメーション再生しているかどうか
    bool IsPlayAnimation() const
    {
        return !(this->isAnimationFinished);
    }

    // 使用例
    // modelComponent->SetAnimationClip(
    void SetAnimationClip(const std::string& animationName, const bool loop = false, const bool isBlend = false, const float blendTime = 0.3f)
    {
        this->isAnimationFinished = false;
        //this->animationTime = 0.0f;
        this->animationNextClip = animationNameToIndex_[animationName];
        this->isAnimationLoop = loop;
        this->currentAnimationName = animationName;
        this->transitionTime = blendTime;
        if (isBlend)
        {
            isBlendingAnimation = true;
            transitionState = AnimationController::AnimationTransitionState::NotStarted;
        }
        else
        { // ブレンドしないなら現在のアニメーションを次のアニメーションに変更する
            this->animationClip = animationNameToIndex_[animationName];
            //isBlendingAnimation = false;
            transitionState = AnimationController::AnimationTransitionState::Completed;
        }
    }

    void OnUpdate(const float deltaTime);

    // アニメーションの再生倍率を変更する関数
    void SetAnimationRate(const float animationRate) { this->animationRate = animationRate; }

    // アニメーションを止める処理
    void Stop()
    {
        isAnimationFinished = true;
        transitionState = AnimationTransitionState::NotStarted;
    }

    // アニメーションのループを切りよく終了させるフラグ
    void RequestStopLoop()
    {
        requestStopLoop = true;
    }

    void DrawImGui();
private:
    SkeletalMeshComponent* target_ = nullptr;

    std::unordered_map<std::string, size_t> animationNameToIndex_;

    // アニメーションブレンドに使用するノード
    std::vector<InterleavedGltfModel::Node> animationNodes[2];
    std::vector<InterleavedGltfModel::Node> blendAnimationNodes;

    enum class AnimationTransitionState :uint8_t
    {
        NotStarted,
        Inprogress,
        Completed,
    };

    //遷移ステート
    AnimationTransitionState transitionState = AnimationTransitionState::NotStarted;

    //アニメーションの再生倍率
    float animationRate = 1.0f;     //デフォルト 1,0f

    //アニメーション時間
    float animationTime = 0.0f;

    // 今再生しているアニメーションのインデックス
    size_t animationClip = 0;

    // 次再生したいアニメーションのインデックス
    size_t animationNextClip = 0;

    // アニメーションをループするか
    bool isAnimationLoop = true;

    // 現在のブレンドの比率
    float blendFactor = 0.0f;

    // ブレンド中かどうか
    bool isBlendingAnimation = false;

    // ブレンドしている時間
    float transitionTime = 0.0f;

    // アニメーションが終了したかどうか
    bool isAnimationFinished = false;

    // ループ終了フラグ 
    bool requestStopLoop = false; // 切りよくループを終わらせる

    // 今再生しているアニメーションの名前
    std::string currentAnimationName = "";
};

#endif  //ANIMATION_CONTROLLER_H