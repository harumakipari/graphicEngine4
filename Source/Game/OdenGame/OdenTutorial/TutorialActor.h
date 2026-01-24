#pragma once

#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

class OdenIngredientActor;
class TutorialManager;

class TutorialActor : public Actor
{
public:
    TutorialActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~TutorialActor() = default;
    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;
    void DrawImGuiDetails() override;

    // チュートリアルマネージャーを取得
    class TutorialManager* GetTutorialManager() const { return tutorialManager.get(); }

    // チュートリアル開始処理
    void StartTutorial();

    // 今掴まれている食材を入れる
    void OnIngredientGrabbed(const std::shared_ptr<Actor>& ingredient);

    // 今掴まれている食材を伝える関数
    std::shared_ptr<OdenIngredientActor> GetGrabbedIngredient() const;

    // 吹き出しを表示する　ダイコンのチュートリアル用
    void ShowBalloonNearIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient);

    // 吹き出しを表示する　丸っぽいのチュートリアル用
    void ShowCircleBallonNearIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient);

    // 吹き出しや文字を全て非表示にする関数
    void HideAllBubbles();

private:

private:
    std::unique_ptr<TutorialManager> tutorialManager;
    std::weak_ptr<OdenIngredientActor> grabbedIngredient;

    std::shared_ptr<UIImageComponent> bubbleUpImage;
    std::shared_ptr<UIImageComponent> bubbleDownImage;
    std::shared_ptr<UIImageComponent> bubbleLeftImage;
    std::shared_ptr<UIImageComponent> bubbleRightImage;


    std::shared_ptr<UIImageComponent> thisKonnyakuImage;
    std::shared_ptr<UIImageComponent> thisEggImage;
    std::shared_ptr<UIImageComponent> thisKobumusubiImage;
    std::shared_ptr<UIImageComponent> thisGobotenImage;
    std::shared_ptr<UIImageComponent> thisChikuwaImage;


    std::shared_ptr<UIImageComponent> thisNotCircleImage;

    XMFLOAT2 uiOffsetPos = { 0.0f,0.0f };
    XMFLOAT2 uiTextOffsetPos = { 110.0f,15.0f };
};