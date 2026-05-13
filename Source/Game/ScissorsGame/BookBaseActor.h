#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"

class BookBaseActor :public Actor
{
protected:
    enum class BookPageState : uint8_t
    {
        Closed,        // 閉じてる
        FirstPage,
        SecondPage
    };

    struct StageSelectData  // ステージ選択のデータ
    {
        STAGE_NAME stage;
        std::shared_ptr<SkeletalMeshComponent> model;
        std::shared_ptr<BoxComponent> collider;
        DirectX::XMFLOAT3 offsetPos; // 最初のオフセットデータ
    };

    struct BookPage // 本のページ
    {
        std::string parentName;
        std::vector<StageSelectData> stages;
    };
public:
    explicit BookBaseActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // 本を開く
    void OpenBook(float interval);

    // 本を閉じる処理
    void CloseBook(float interval);

    // 二ページ目を開く処理
    void OpenSecondPage(float interval);

    // 二ページ目を戻す処理
    void CloseSecondPage(float interval);

protected:
    // 最初の本の状態を設定する
    void SetInitPageState(BookPageState initialState);

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


    std::unique_ptr<EasingRunner> easingOneRunner;  // 一ページ目のeasing
    std::unique_ptr<EasingRunner> easingTwoRunner;  // 二ページ目のeasing

    bool isAnimating = false;// 演出中かどうか

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

    // 真ん中のページを開くときの角度
    float openFirstPageAngle = 0.0f;
    float closeFirstPageAngle = -180.0f;
    // 真ん中のページを開くときのワッペンの位置
    float openPatchPosY = 0.0f;
    float closePatchPosY = -0.2f;

    BookPageState bookState = BookPageState::Closed;    // 本の状態

    // 調整
    float firstRate = 0.2f; // 本を閉じる時の最初のページの割合
    float secondRate = 0.8f;   // 本を閉じる時の二枚目のページの割合
};
