#pragma once

#include "EnemyScoreData.h"
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"
#include "TutorialManager.h"

class EnemyBase;
class ScissorsPlayer1;
class TutorialManager;

struct TutorialTarget
{
    std::weak_ptr<EnemyBase> enemy;
    std::shared_ptr<UIImageComponent> arrowImage;
};

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

    // プレイヤーを取得する
    ScissorsPlayer1* GetPlayer();

    // チュートリアルターゲットに登録する
    void AddTutorialTarget(const std::shared_ptr<EnemyBase>& enemy);

    // チュートリアルターゲットを取得する
    std::vector<TutorialTarget> GetTutorialTargets() { return tutorialTargets; }

    // 敵を生成する
    std::shared_ptr<EnemyBase> SpawnEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 });

    // 縫い留められた敵がいるかどうか
    bool HasPinnedEnemy() const;

    // 倒された敵がいるかどうか
    bool HasDeadEnemy()const;
private:
    // 敵の上に出す矢印
    void UpdateShowArrowEnemy();

private:
    XMFLOAT2 arrowOffsetPos = { 8.0f,40.0f };

    std::unique_ptr<TutorialManager> tutorialManager;
    std::vector<TutorialTarget> tutorialTargets;
};