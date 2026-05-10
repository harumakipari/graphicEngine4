#pragma once

#include "EnemyScoreData.h"
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"
#include "TutorialManager.h"

class EnemyBase;
class ScissorsPlayer1;
class TutorialManager;

struct TutorialTargetEnemy
{
    std::weak_ptr<EnemyBase> enemy;
    std::shared_ptr<UIImageComponent> arrowImage;
};

// チュートリアルで遅延してスポーンさせる
struct PendingTutorialSpawn
{
    XMFLOAT3 position;

    YarnEnemyType type = YarnEnemyType::Static;

    bool isBig = false;

    float speed = 2.0f;

    XMFLOAT3 direction = { 1,0,0 };

    float timer = 0.0f;

    bool previewed = false;
    bool spawned = false;

    bool isTied = false;
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
    void AddTutorialEnemy(const std::shared_ptr<EnemyBase>& enemy);

    // チュートリアルターゲットを取得する
    std::vector<TutorialTargetEnemy> GetTutorialEnemies() { return tutorialTargets; }

    // 敵を生成する
    std::shared_ptr<EnemyBase> SpawnEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 }, bool isTied = false);

    // 縫い留められた敵がいるかどうか
    bool HasPinnedEnemy() const;

    // 倒された敵がいるかどうか
    bool HasDeadEnemy()const;

    // チュートリアルターゲットをクリアする
    void ClearTutorialTargets();

    // 矢印を出す
    void ShowArrows();

    // 矢印を消す
    void HideArrows();

    // 予約スポーンをする
    void ReserveSpawnEnemy(){}

private:
    // 敵の上に出す矢印
    void UpdateShowArrowEnemy(float deltaTime);

    // 壁際に出す矢印
    void UpdateSideArrow(float deltaTime);

private:
    XMFLOAT2 arrowOffsetPos = { 8.0f,40.0f };

    std::unique_ptr<TutorialManager> tutorialManager;
    std::vector<TutorialTargetEnemy> tutorialTargets;

    std::shared_ptr<UIImageComponent> arrowRightComponent;
    std::shared_ptr<UIImageComponent> arrowLeftComponent;
    std::shared_ptr<UIImageComponent> arrowUpComponent;
    std::shared_ptr<UIImageComponent> arrowDownComponent;

    std::vector<PendingTutorialSpawn> pendingTutorialSpawns;    // チュートリアル遅延湧き

    float elapsedTime = 0.0f;
};