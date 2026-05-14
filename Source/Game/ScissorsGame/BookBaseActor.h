#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

class BookBaseActor :public Actor
{
protected:
    enum class BookPageState : uint8_t
    {
        Closed,        // 閉じてる
        OpeningBook,    // 本を空けている途中
        FirstPage,  // 一ページ目が開いている
        OpeningSecondPage,  // 二ページ目を空けている途中
        SecondPage, // 二ページ目が空いている
        ReturningFirstPage,    // 一ページ目へ戻っている途中
        ClosingBook,   // 本を閉じている途中
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
        std::vector<std::shared_ptr<StageSelectData>> stages;
    };

    struct BookPageButtons
    {
        std::shared_ptr<UIButtonComponent> left;
        std::shared_ptr<UIButtonComponent> right;

        void SetEnable(bool enable)
        {
            if (left)
            {
                left->SetEnable(enable);
                left->SetVisible(enable);
            }

            if (right)
            {
                right->SetEnable(enable);
                right->SetVisible(enable);
            }
        }
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

    // モデルを生成する
    void CreateBookModel(const std::string& backCoverModelName, const std::string& middleModelName);
private:
    // ステージパッチを生成する
    void CreateStagePatch(BookPage& page,
        STAGE_NAME stage,
        const char* modelPath,
        const DirectX::XMFLOAT3& pos);

    // ページのパッチの更新処理
    void UpdatePage(BookPage& page);

    // 本を閉じている時の処理
    void UpdateClosedBook();

    // コントローラー対応の本が開く処理
    void HandlePadInput();

    // コントローラー対応用ステージ選択の処理
    void HandlePadStageSelection(float deltaTime);

    // コントローラー対応時に選択切り替え処理
    void MoveSelection(int dir);

public:
    // 本が押されたことを通知する
    std::function<void()> onRequestOpenBook;

protected:
    BookPage leftPage;
    BookPage rightPage;

    std::string backCoverName;  // 裏表紙の名前

    // UIのボタン
    BookPageButtons firstButtons;
    BookPageButtons secondButtons;

    BookPageState bookState = BookPageState::Closed;    // 本の状態

    // コントローラー対応時に使用するステージごとの配列
    std::vector<std::shared_ptr<StageSelectData>> selectableStages;
    int selectedStageIndex = 0;

private:

    std::shared_ptr<SkeletalMeshComponent> bookLeftModel;
    std::shared_ptr<SkeletalMeshComponent> bookRightModel;
    std::shared_ptr<SkeletalMeshComponent> bookMiddleModel; // 本の真ん中のモデル
    std::shared_ptr<SkeletalMeshComponent> bookSpineModel;  // 背表紙モデル

    std::shared_ptr<BoxComponent> bookSpineCollisionComponent;  // 背表紙の当たり判定


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

    std::shared_ptr<UIImageComponent> startAButton;   // ワッペンの近くにAボタンを表示する

    // 裏表紙のモデル名
    std::string backCoverModelName = "./Data/TeamModels/Title/BookRight.gltf";
    // 真ん中のモデル名
    std::string middleModelName = "./Data/TeamModels/Title/BookMiddle.gltf";


    // 調整
    float firstRate = 0.2f; // 本を閉じる時の最初のページの割合
    float secondRate = 0.8f;   // 本を閉じる時の二枚目のページの割合
    DirectX::XMFLOAT3 patchAButtonOffset = { -0.5f,0.0f,0.5f, };    
};
