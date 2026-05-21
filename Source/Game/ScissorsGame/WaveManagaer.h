#pragma once
#include "StageData.h"
#include "Core/Actor.h"

#include "Components/Effect/ParticleComponent.h"
#include "EnemyBase.h"


class WaveManager :public Actor
{
public:
    enum class WaveState :uint8_t
    {
        Ready,      // 3,2,1カウント中
        WaitingNextWave, // 次のwaveまでの待ち時間
        Spawning,   // 通常のWave処理
        Finished
    };
public:
    explicit WaveManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    // 指定したステージのウェーブを設定
    void SetWaves(STAGE_NAME stageId);

    void Update(float deltaTime)override;

    bool AllEnemiesDead() const
    {
        return enemyCount == 0;
    }

    bool TutorialAllEnemiesDead() const
    {
        return spawnCount == 0;
    }

    // ボビンのチュートリアルで使用するwaveが始まったかどうか
    bool IsWaveStarted() { return waveStarted; }

    // 今のwaveを取得する
    int GetCurrentWave() { return currentWave; }
private:
    // 最初の待ち更新処理
    void UpdateReady(float deltaTime);

    // wave 間の更新処理
    void UpdateWaiting(float deltaTime);

    // 敵スポーンの更新処理
    void UpdateSpawning(float deltaTime);

    // 次のwaveに行くときの処理
    void GoToNextWave();

    // 敵を生成する
    void SpawnEnemy(
        const DirectX::XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed , const DirectX::XMFLOAT3& dir,bool isTied);



    // 出現エフェクトを生成
    void SpawnPreviewEffect(DirectX::XMFLOAT3 pos);

    // 敵が死んだときに呼ぶ関数として登録する関数
    void OnDeath(EnemyBase* enemy)
    {
        spawnCount--;
        enemyCount--;
        killCount++;

        aliveEnemies.erase(
            std::remove_if(aliveEnemies.begin(), aliveEnemies.end(),
                [enemy](const std::weak_ptr<EnemyBase>& weakEnemy)
                {
                    if (auto e = weakEnemy.lock())
                    {
                        return e.get() == enemy;
                    }
                    return false;
                }),
            aliveEnemies.end());
    }

    // ステージ全体の最後のWaveの、最後の1体
    void OnLastEnemySpawned();

    // 必要ならボスを生成する
    void SpawnBossIfNeeded(const StageData& stageData) const;

    // ゲーム終了を通知する関数
    void RequestGameClear();
public:
    std::vector<std::weak_ptr<EnemyBase>> aliveEnemies; // 生き残っている敵

private:
    int currentWave = 0;  // 今のウェーブ
    float timer = 0.0f;
    int spawnIndex = 0;

    std::vector<WaveData> waves;
    std::vector<SpawnRuntime> spawnStates;

    int enemyCount = 0;
    int spawnCount = 0;
    int killCount = 0; // キルカウント

    std::shared_ptr<ParticleComponent> spawnEffectComponent; // 出現エフェクト用コンポーネント

    WaveState waveState = WaveState::Ready;
    bool hasSpawnedAnyEnemy = false; //敵がスポーンを開始したかどうか
    float startTimer = 0.0f;// wave１が始まるまでの時間

    float lineCheckTimer = 0.0f;


    bool hasTriggeredLastSpawn = false;

    bool hasBossStage = false;  // ボスステージかどうか
    bool hasEndedGame = false;  // ゲームを終了終了条件を満たしているかどうか
    bool waveStarted = false;


    bool isClearSlowPlaying = false;
    float clearSlowTimer = 0.0f;
};