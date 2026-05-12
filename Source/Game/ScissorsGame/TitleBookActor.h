#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"

// タイトルステージモデルアクター
class TitleBookActor :public Actor
{
    struct StageSelectData  // ステージ選択のデータ
    {
        STAGE_NAME stage;
        std::shared_ptr<SkeletalMeshComponent> model;
        std::shared_ptr<BoxComponent> collider;
    };
    struct BookPage // 本のページ
    {
        std::string parentName;
        std::vector<StageSelectData> stages;
    };

public:
    explicit TitleBookActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // 本を開く
    void Play(float interval);

    // 本を閉じる
    void PlayReverse(float interval);

private:
    // ステージパッチを生成する
    void CreateStagePatch(BookPage& page,
        STAGE_NAME stage,
        const char* modelPath,
        const DirectX::XMFLOAT3& pos);

    // ページのパッチの更新処理
    void UpdatePage(BookPage& page);

private:
    BookPage leftPage;
    BookPage rightPage;

    std::shared_ptr<SkeletalMeshComponent> bookLeftModel;
    std::shared_ptr<SkeletalMeshComponent> bookRightModel;
    std::shared_ptr<SkeletalMeshComponent> bookMiddleModel; // 本の真ん中のモデル
    std::shared_ptr<SkeletalMeshComponent> bookSpineModel;  // 背表紙モデル

    std::shared_ptr<SkeletalMeshComponent> patchTutorialModel;

    std::unique_ptr<EasingRunner> easingRunner; 

    std::unique_ptr<EasingRunner> easingSpinePosRunner; // 背表紙の position
    std::unique_ptr<EasingRunner> easingSpineAngleRunner; // 背表紙の角度

    float startEuler = 0.0f;
    float endEuler = 180.0f;

    float startSpineEuler = 0.0f;
    float endSpineEuler = -90.0f;

    float startSpinPosY = 0.0f;
    float endSpinPosY = -0.2f;

    float angle = 0.0f;
};

