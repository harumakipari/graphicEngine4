#pragma once
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
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

    // この種類の食材を掴むことができるか
    virtual ETutorialIngredientResult CanGrabIngredient(EOdenType ingredientType) const
    {
        return ETutorialIngredientResult::Allow;
    }

    // 掴んではいけない食材の時の処理
    virtual void OnDeniedGrab(std::shared_ptr<Actor> ingredient) {}

    // 掴める食材の時の処理
    virtual void OnAllowGrab(std::shared_ptr<Actor> ingredient) {}

    // 食材をスワップできるかどうか
    virtual bool CanSwapIngredient() const { return false; }
protected:
    void UpdateMouseClickBlink(float deltaTime);

    void ShowMouseClick(bool visible);

    void ResetMouseClickBlink();
protected:
    TutorialActor* owner = nullptr;

    std::shared_ptr<UIImageComponent> tutorialMouseClickImage;
    std::shared_ptr<UIImageComponent> tutorialMouseClickOffImage;

    XMFLOAT2 imagePos = { 1080.0f,78.0f };
    XMFLOAT2 imageSize = { 700.0f,410.0f };

    float mouseBlinkTimer = 0.0f;
    float mouseBlinkInterval = 0.6f; // 切り替え間隔
    bool isMouseClickOn = false;
};

// チュートリアルステップ  ：おでん屋さんを始める
class TutorialStep_StartOdenStore : public TutorialStep
{
public:
    TutorialStep_StartOdenStore(TutorialActor* actor);
    virtual ~TutorialStep_StartOdenStore();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "StartOdenStore"; }
private:
    std::shared_ptr<UIImageComponent> tutorialStartStoreImage;
};


// チュートリアルステップ  ：おでんの具材を取る
class TutorialStep_TakeOdenIngredient : public TutorialStep
{
public:
    enum class Phase :uint8_t
    {
        WaitGrabDaikon,
        HoldDaikon,
        HoverOrder,
    };
public:
    TutorialStep_TakeOdenIngredient(TutorialActor* actor);
    virtual ~TutorialStep_TakeOdenIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TakeOdenIngredient"; }

    // この種類の食材を掴むことができるか
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType) const override
    {
        if (ingredientType == EOdenType::Daikon)
            return ETutorialIngredientResult::Allow;

        return ETutorialIngredientResult::DenyNotTarget;
    }

    // 掴んではいけない食材の時の処理
    void OnDeniedGrab(std::shared_ptr<Actor> ingredient) override;

    // 掴める食材の時の処理
    void OnAllowGrab(std::shared_ptr<Actor> ingredient) override;

private:
    std::shared_ptr<UIImageComponent> tutorialTakeIngredientImage;
    std::shared_ptr<UIImageComponent> tutorialSubmitIngredientImage;

    std::shared_ptr<UIImageComponent> tutorialBubbleLeftImage;
    std::shared_ptr<UIImageComponent> tutorialReleaseMouseImage;

    Phase phase = Phase::WaitGrabDaikon;
};


// チュートリアルステップ  ：　提出クリアした
class TutorialStep_SubmitClearIngredient : public TutorialStep
{
public:
    TutorialStep_SubmitClearIngredient(TutorialActor* actor);
    virtual ~TutorialStep_SubmitClearIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "SubmitOdenClear"; }


private:
    std::shared_ptr<UIImageComponent> tutorialClearSubmitIngredientImage;
};


// チュートリアルステップ  ：　●のおでんがほしい客が来る ダイコンを渡す
class TutorialStep_SubmitOdenCircleIngredient : public TutorialStep
{
public:
    TutorialStep_SubmitOdenCircleIngredient(TutorialActor* actor);
    virtual ~TutorialStep_SubmitOdenCircleIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "SubmitOdenCircleIngredient"; }


private:
    std::shared_ptr<UIImageComponent> tutorialSubmitIngredientImage;
    std::shared_ptr<UIImageComponent> tutorialAnywayDaikonImage;
};


// チュートリアルステップ  ：　丸いのを成功した後に出す　
class TutorialStep_ClearCircleIngredient : public TutorialStep
{
public:
    TutorialStep_ClearCircleIngredient(TutorialActor* actor);
    virtual ~TutorialStep_ClearCircleIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ClearCircleIngredient"; }


private:
    std::shared_ptr<UIImageComponent> tutorialClearCircleIngredientImage;
};


// チュートリアルステップ  ：　ダイコンを回す
class TutorialStep_RotateOdenIngredient : public TutorialStep
{
public:
    TutorialStep_RotateOdenIngredient(TutorialActor* actor);
    virtual ~TutorialStep_RotateOdenIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "RotateOdenIngredient"; }
private:
    std::shared_ptr<UIImageComponent> tutorialRotateOdenImage;
};