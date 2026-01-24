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
    virtual ETutorialIngredientResult CanGrabIngredient(EOdenType ingredientType, EOdenShapeCategory shape ) const
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

    XMFLOAT2 imagePos = { 1080.0f,78.0f };
    XMFLOAT2 imageSize = { 700.0f,410.0f };

    float mouseBlinkTimer = 0.0f;
    float mouseBlinkInterval = 0.6f; // 切り替え間隔
    bool isMouseClickOn = false;
    bool isUpdateMouse = true;
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

    // この種類の食材を掴むことができるか
    virtual ETutorialIngredientResult CanGrabIngredient(EOdenType ingredientType, EOdenShapeCategory shape) const
    {
        return ETutorialIngredientResult::DenyNotTarget;
    }

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
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType, EOdenShapeCategory shape) const override
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


// チュートリアルステップ  ：　●のおでんがほしい客が来る
class TutorialStep_ComeOdenCircleIngredient : public TutorialStep
{
public:
    TutorialStep_ComeOdenCircleIngredient(TutorialActor* actor);
    virtual ~TutorialStep_ComeOdenCircleIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ComeOdenCircleIngredient"; }

    // この種類の食材を掴むことができるか
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType, EOdenShapeCategory shape) const override
    {
        return ETutorialIngredientResult::DenyNotTarget;
    }

private:
    std::shared_ptr<UIImageComponent> tutorialSubmitIngredientImage;
};


// チュートリアルステップ  ：　●のおでんを渡す
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

    // この種類の食材を掴むことができるか
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType, EOdenShapeCategory shape) const override
    {
        if (shape == EOdenShapeCategory::RoundLike)
        {// 丸いおでんはつかめる
            return ETutorialIngredientResult::Allow;
        }

        return ETutorialIngredientResult::DenyNotTarget;
    }

    // 掴んではいけない食材の時の処理
    void OnDeniedGrab(std::shared_ptr<Actor> ingredient) override;

    // 掴める食材の時の処理
    void OnAllowGrab(std::shared_ptr<Actor> ingredient) override;

private:
    std::shared_ptr<UIImageComponent> tutorialSubmitCircleIngredientImage;
    std::shared_ptr<UIImageComponent> tutorialAnywayDaikonImage;
    std::shared_ptr<UIImageComponent> tutorialReleaseMouseImage;
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


// チュートリアルステップ  ：　回る
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


// チュートリアルステップ  ：　四角っぽいお客さんが来る
class TutorialStep_ComeOdenSquareIngredient : public TutorialStep
{
public:
    TutorialStep_ComeOdenSquareIngredient(TutorialActor* actor);
    virtual ~TutorialStep_ComeOdenSquareIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ComeOdenSquareIngredient"; }

    // この種類の食材を掴むことができるか
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType, EOdenShapeCategory shape) const override
    {
        return ETutorialIngredientResult::DenyNotTarget;
    }

private:
    std::shared_ptr<UIImageComponent> tutorialSubmitIngredientImage;
};


// チュートリアルステップ  ：　四角のおでんを渡す
class TutorialStep_SubmitOdenSquareIngredient : public TutorialStep
{
public:
    TutorialStep_SubmitOdenSquareIngredient(TutorialActor* actor);
    virtual ~TutorialStep_SubmitOdenSquareIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "SubmitOdenSquareIngredient"; }

    // この種類の食材を掴むことができるか
    ETutorialIngredientResult CanGrabIngredient(const EOdenType ingredientType, EOdenShapeCategory shape) const override
    {
        if (shape == EOdenShapeCategory::SquareLike)
        {// 四角いおでんはつかめる
            return ETutorialIngredientResult::Allow;
        }

        return ETutorialIngredientResult::DenyNotTarget;
    }

    // 掴んではいけない食材の時の処理
    void OnDeniedGrab(std::shared_ptr<Actor> ingredient) override;

    // 掴める食材の時の処理
    void OnAllowGrab(std::shared_ptr<Actor> ingredient) override;

private:
    std::shared_ptr<UIImageComponent> tutorialSubmitCircleIngredientImage;
    std::shared_ptr<UIImageComponent> tutorialAnywayDaikonImage;
    std::shared_ptr<UIImageComponent> tutorialReleaseMouseImage;
};

// チュートリアルステップ  ：　四角いのを成功した後に出す　
class TutorialStep_ClearSquareIngredient : public TutorialStep
{
public:
    TutorialStep_ClearSquareIngredient(TutorialActor* actor);
    virtual ~TutorialStep_ClearSquareIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ClearSquareIngredient"; }


private:
    std::shared_ptr<UIImageComponent> tutorialClearCircleIngredientImage;
};


// チュートリアルステップ  ： 下の段のおでんのスワップについて説明　
class TutorialStep_SwapIngredient : public TutorialStep
{
public:
    TutorialStep_SwapIngredient(TutorialActor* actor);
    virtual ~TutorialStep_SwapIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "SwapIngredient"; }

    // 食材をスワップできるかどうか
    bool CanSwapIngredient() const override{ return true; }

private:
    std::shared_ptr<UIImageComponent> tutorialClearCircleIngredientImage;
};


// チュートリアルステップ  ： スワップした後の
class TutorialStep_ClearSwapIngredient : public TutorialStep
{
public:
    TutorialStep_ClearSwapIngredient(TutorialActor* actor);
    virtual ~TutorialStep_ClearSwapIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ClearSwapIngredient"; }

    // 食材をスワップできるかどうか
    bool CanSwapIngredient() const override { return true; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
};

// チュートリアルステップ  ： 他の形の紹介
class TutorialStep_IntroduceShape : public TutorialStep
{
public:
    TutorialStep_IntroduceShape(TutorialActor* actor);
    virtual ~TutorialStep_IntroduceShape();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "IntroduceShape"; }

    // 食材をスワップできるかどうか
    bool CanSwapIngredient() const override { return true; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
};

// チュートリアルステップ  ： 開店準備OKです！
class TutorialStep_ClearTutorial : public TutorialStep
{
public:
    TutorialStep_ClearTutorial(TutorialActor* actor);
    virtual ~TutorialStep_ClearTutorial();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "ClearTutorial"; }

    // 食材をスワップできるかどうか
    bool CanSwapIngredient() const override { return true; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
};

