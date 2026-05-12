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

    std::unique_ptr<EasingRunner> easingOneRunner;  // 一ページ目のeasing
    std::unique_ptr<EasingRunner> easingTwoRunner;  // 二ページ目のeasing



    // 背表紙の角度
    float openSpineEuler = 0.0f;    
    float closeSpineEuler = -90.0f;

    // 背表紙の位置
    float openSpinPosY = 0.0f;
    float closeSpinPosY = -0.2f;


    float bookOneAlpha = 0.0f; // １ページ目
    float bookTwoAlpha = 0.0f; // ２ページ目


    float openBookAngle = 0.0f;
    float closeBookAngle = 180.0f;
};

