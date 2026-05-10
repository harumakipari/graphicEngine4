#pragma once
#include "UI/Widgets/Widget.h"

class TutorialActor;

class TutorialStep
{
public:
    TutorialStep(TutorialActor* actor);
    virtual ~TutorialStep() = default;

    // コピー禁止（オブジェクトの重複を防ぐ）
    TutorialStep(const TutorialStep&) = delete;
    TutorialStep& operator =(const TutorialStep&) = delete;
    TutorialStep(TutorialStep&&) noexcept = delete;
    TutorialStep& operator =(TutorialStep&&) noexcept = delete;

    // ステートに入った時のメソッド
    virtual void Enter() = 0;

    // ステートで実行するメソッド
    virtual void Execute(float deltaTime) = 0;

    // ステージから出ていくときのメソッド
    virtual void Exit() = 0;

    virtual const char* GetName() const = 0;

protected:
    void UpdateMouseClickBlink(float deltaTime);

    void ShowMouseClick(bool visible);

    void ResetMouseClickBlink();

    // マウスをもう見せない
    void NotShowMouse()
    {
        isUpdateMouse = false;
        tutorialMouseClickImage->SetVisible(false);
        tutorialMouseClickOffImage->SetVisible(false);
    }
protected:
    TutorialActor* owner = nullptr;

    std::shared_ptr<UIImageComponent> tutorialMouseClickImage;
    std::shared_ptr<UIImageComponent> tutorialMouseClickOffImage;

    XMFLOAT2 imagePos = { 1080.0f,18.0f };
    XMFLOAT2 imageSize = { 840.0f,492.0f };

    float mouseBlinkTimer = 0.0f;
    float mouseBlinkInterval = 0.6f; // 切り替え間隔
    bool isMouseClickOn = false;
    bool isUpdateMouse = true;

};

// チュートリアルステップ : WASD で移動or 左スティックで移動！
class TutorialStep_MoveStart : public TutorialStep
{
public:
    TutorialStep_MoveStart(TutorialActor* actor);
    virtual ~TutorialStep_MoveStart();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_MoveStart"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
    bool startWalk = false; // 歩き始めたかどうか
   const float toNextStepInterval = 1.0f; // 次のステップに行くまでの時間
};

// チュートリアルステップ : // 「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
class TutorialStep_ChargeStart : public TutorialStep
{
public:
    TutorialStep_ChargeStart(TutorialActor* actor);
    virtual ~TutorialStep_ChargeStart();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_ChargeStart"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
    bool startDash = false; // ダッシュし始めたかどうか
    const float toNextStepInterval = 2.0f; // 次のステップに行くまでの時間
};

// チュートリアルステップ : // 「左クリックを離すと、ぬいダッシュ！」
class TutorialStep_SpawnStaticEnemy : public TutorialStep
{
public:
    TutorialStep_SpawnStaticEnemy(TutorialActor* actor);
    virtual ~TutorialStep_SpawnStaticEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_SpawnStaticEnemy"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
    bool startDash = false; // ダッシュし始めたかどうか
    const float toNextStepInterval = 2.0f; // 次のステップに行くまでの時間
};

// チュートリアルステップ : // 「敵をぬいとめたよ！」「ぬいとめた敵に もう一度ぬいダッシュ！」
class TutorialStep_TiedEnemy : public TutorialStep
{
public:
    TutorialStep_TiedEnemy(TutorialActor* actor);
    virtual ~TutorialStep_TiedEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_TiedEnemy"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};