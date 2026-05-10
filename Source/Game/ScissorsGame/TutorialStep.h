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

    // ダッシュできるかどうか
    virtual bool CanDash() { return true; }

    // 反射した
    void IsUseRedirect(bool useDirectFlag) { isUseRedirect = useDirectFlag; }

    // 跳ね返りで敵を倒したことを通知する
    void SetRedirectKillEnemy(bool isRedirectKillEnemy) { this->isRedirectKillEnemy = isRedirectKillEnemy; }

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

    XMFLOAT2 imagePos = { 330.0f,6.0f };
    XMFLOAT2 imageSize = { 600.0f,275.0f };

    float mouseBlinkTimer = 0.0f;
    float mouseBlinkInterval = 0.6f; // 切り替え間隔
    bool isMouseClickOn = false;
    bool isUpdateMouse = true;
    bool isUseRedirect = false;// 反射したかどうか
    bool isRedirectKillEnemy = false;// 反射で敵を倒したかどうか

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

// チュートリアルステップ : // 「いいね！敵は時間がたつと ぬいとめがほどけちゃうよ！」
class TutorialStep_NiceAttackEnemy : public TutorialStep
{
public:
    TutorialStep_NiceAttackEnemy(TutorialActor* actor);
    virtual ~TutorialStep_NiceAttackEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_NiceAttackEnemy"; }

    // ダッシュできるかどうか
    bool CanDash() override { return false; }


private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : 「次は動く敵をぬいとめよう！」
class TutorialStep_SpawnMoveEnemy : public TutorialStep
{
public:
    TutorialStep_SpawnMoveEnemy(TutorialActor* actor);
    virtual ~TutorialStep_SpawnMoveEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_SpawnMoveEnemy"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : 「ぬいとめると 動きが止まるよ！そのまま もう一度ぬいダッシュ！」
class TutorialStep_TiedMoveEnemy : public TutorialStep
{
public:
    TutorialStep_TiedMoveEnemy(TutorialActor* actor);
    virtual ~TutorialStep_TiedMoveEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_TiedMoveEnemy"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : 「ぬいダッシュしていない時に敵にぶつかると ハートが減っちゃうよ！」
class TutorialStep_DecreaseHeart : public TutorialStep
{
public:
    TutorialStep_DecreaseHeart(TutorialActor* actor);
    virtual ~TutorialStep_DecreaseHeart();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_DecreaseHeart"; }

    // ダッシュできるかどうか
    bool CanDash() override { return false; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : 今度は布の端に向かってぬいダッシュ！
class TutorialStep_DashClothSide : public TutorialStep
{
public:
    TutorialStep_DashClothSide(TutorialActor* actor);
    virtual ~TutorialStep_DashClothSide();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_DashClothSide"; }


private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};


// チュートリアルステップ : 布の端で”ぬい返り”するよ！
class TutorialStep_DashRedirect : public TutorialStep
{
public:
    TutorialStep_DashRedirect(TutorialActor* actor);
    virtual ~TutorialStep_DashRedirect();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_DashRedirect"; }

    // ダッシュできるかどうか
    bool CanDash() override { return false; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : ぬい返りで敵を倒してみよう！
class TutorialStep_AttackEnemyRedirect : public TutorialStep
{
public:
    TutorialStep_AttackEnemyRedirect(TutorialActor* actor);
    virtual ~TutorialStep_AttackEnemyRedirect();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_AttackEnemyRedirect"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ : ぬい返りで倒すと高スコア！
class TutorialStep_RedirectHighScore : public TutorialStep
{
public:
    TutorialStep_RedirectHighScore(TutorialActor* actor);
    virtual ~TutorialStep_RedirectHighScore();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_RedirectHighScore"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};

// チュートリアルステップ :まとめて倒してみよう！
class TutorialStep_AttackAllEnemy : public TutorialStep
{
public:
    TutorialStep_AttackAllEnemy(TutorialActor* actor);
    virtual ~TutorialStep_AttackAllEnemy();
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステージから出ていくときのメソッド
    void Exit() override;
    virtual const char* GetName() const override { return "TutorialStep_AttackAllEnemy"; }

private:
    std::shared_ptr<UIImageComponent> tutorialImage;
    float elapsedTime = 0.0f;
};
