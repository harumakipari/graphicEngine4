#pragma once

#include "EnemyScoreData.h"
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"
#include "TutorialManager.h"
#include "Components/Effect/ParticleComponent.h"

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

    bool isTargetTutorial = false;
    bool spawnArrowUi = true;

    float previewStartTime = 0.5f; // 予告が始まるまでの時間
    float spawnTime = 1.5f; // 出現までの時間

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

    // チュートリアルターゲットを取得する
    std::vector<TutorialTargetEnemy> GetTutorialEnemies() { return tutorialTargets; }

    // 縫い留められた敵がいるかどうか
    bool HasPinnedEnemy() const;

    // 倒された敵がいるかどうか
    bool HasDeadEnemy()const;

    // チュートリアルターゲットをクリアする
    void ClearTutorialTargets();

    // チュートリアルターゲットの矢印を非表示にする
    void HideTutorialTargetsArrows();

    // 矢印を出す
    void ShowArrows();

    // 矢印を消す
    void HideArrows();

    // 予約スポーンをする
    void ReserveSpawnEnemy(const XMFLOAT3& pos,
        YarnEnemyType type,
        bool isBig,
        float speed,
        const XMFLOAT3& dir,
        bool isTied, bool isTargetTutorial, bool spawnArrowUi = true, float previewTime = 0.5f, float spawnTime = 1.5f);

    // 敵の数を取得する
    int GetEnemyCount() { return enemyCount; }

    // 生き残っている敵
    int GetAliveEnemyCount() const;

    // 予約している敵の数を取得する
    int GetPendingSpawnCount() const
    {
        return static_cast<int>(pendingTutorialSpawns.size());
    }
private:
    // 敵の上に出す矢印
    void UpdateShowArrowEnemy(float deltaTime);

    // 壁際に出す矢印
    void UpdateSideArrow(float deltaTime);

    // 敵を生成する
    std::shared_ptr<EnemyBase> SpawnEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 }, bool isTied = false);

    // 敵の遅延湧きの更新処理
    void UpdatePendingTutorialSpawns(float deltaTime);

    // 出現エフェクトを生成
    void SpawnPreviewEffect(DirectX::XMFLOAT3 pos);

    // チュートリアルターゲットに登録する
    void AddTutorialEnemy(const std::shared_ptr<EnemyBase>& enemy, bool spawnArrowUi);


private:
    XMFLOAT2 arrowOffsetPos = { 8.0f,40.0f };

    std::unique_ptr<TutorialManager> tutorialManager;
    std::vector<TutorialTargetEnemy> tutorialTargets;

    std::shared_ptr<UIImageComponent> arrowRightComponent;
    std::shared_ptr<UIImageComponent> arrowLeftComponent;
    std::shared_ptr<UIImageComponent> arrowUpComponent;
    std::shared_ptr<UIImageComponent> arrowDownComponent;
    std::vector<PendingTutorialSpawn> pendingTutorialSpawns;    // チュートリアル遅延湧き
    std::shared_ptr<ParticleComponent> spawnEffectComponent; // 出現エフェクト用コンポーネント
    float elapsedTime = 0.0f;
    int enemyCount = 0; // 敵の数
};