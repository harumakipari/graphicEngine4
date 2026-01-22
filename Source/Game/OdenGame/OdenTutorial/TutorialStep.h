#pragma once
#include "UI/Widgets/Widget.h"


class TutorialActor;

class TutorialStep
{
public:
    TutorialStep(TutorialActor* actor) :owner(actor) {}
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
    TutorialActor* owner = nullptr;
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
    TutorialStep_TakeOdenIngredient(TutorialActor* actor);
    virtual ~TutorialStep_TakeOdenIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TakeOdenIngredient"; }


private:
    std::shared_ptr<UIImageComponent> tutorialTakeIngredientImage;
    std::shared_ptr<UIImageComponent> tutorialOperateImage;
};


// チュートリアルステップ  ：おでんの具材を提出する
class TutorialStep_SubmitOdenIngredient : public TutorialStep
{
public:
    TutorialStep_SubmitOdenIngredient(TutorialActor* actor);
    virtual ~TutorialStep_SubmitOdenIngredient();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "SubmitOdenIngredient"; }


private:
    std::shared_ptr<UIImageComponent> tutorialSubmitIngredientImage;
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