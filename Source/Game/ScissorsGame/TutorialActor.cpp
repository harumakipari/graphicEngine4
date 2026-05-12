#include "pch.h"
#include "TutorialActor.h"
#include "TutorialStep.h"
#include "Physics/CollisionFunction.h"
#include "TutorialManager.h"
#include "ScissorsPlayer1.h"
#include "TutorialScene.h"
#include "EnemyBase.h"
#include "ScissorsGameState.h"

void TutorialActor::Initialize(const Transform& transform)
{
    std::string parentName = "TutorialActor";

    tutorialManager = std::make_unique<TutorialManager>();
    // 各ステートを登録
    tutorialManager->RegisterState(std::make_unique<TutorialStep_MoveStart>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ChargeStart>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_CancelDash>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SpawnStaticEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_TiedEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_NiceAttackEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SpawnMoveEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_TiedMoveEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_DecreaseHeart>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_DashClothSide>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_DashRedirect>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_AttackEnemyRedirect>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_RedirectHighScore>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_RedirectLongDash>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_AttackAllEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_AttackAllBonus>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_StageClear>(this));

    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    XMFLOAT2 imageSize = { 240.0f,200.0f };

    DirectX::XMFLOAT2 size = { 90.0f,75.0f };

    // 右矢印
    arrowRightComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/arrow_right.png", "arrow_right");
    arrowRightComponent->SetSize({ size.x,size.y });
    arrowRightComponent->SetPivot({ 0.0f,0.5f });
    arrowRightComponent->SetVisible(false);
    uiManager->Add(arrowRightComponent);

    // 左矢印
    arrowLeftComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/arrow_left.png", "arrow_left");
    arrowLeftComponent->SetSize({ size.x,size.y });
    arrowLeftComponent->SetPivot({ 1.0f,0.5f });
    arrowLeftComponent->SetVisible(false);
    uiManager->Add(arrowLeftComponent);

    // 上矢印
    arrowUpComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/arrow_up.png", "arrow_up");
    arrowUpComponent->SetSize({ size.y,size.x });
    arrowUpComponent->SetVisible(false);
    arrowUpComponent->SetPivot({ 0.5f,1.0f });
    uiManager->Add(arrowUpComponent);

    // 下矢印
    arrowDownComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/arrow_down.png", "arrow_down");
    arrowDownComponent->SetSize({ size.y,size.x });
    arrowDownComponent->SetVisible(false);
    arrowDownComponent->SetPivot({ 0.5f,0.0f });
    uiManager->Add(arrowDownComponent);

    // 登場エフェクト用のコンポーネントを追加
    spawnEffectComponent = this->AddComponent<class ParticleComponent>(parentName);
    spawnEffectComponent->Load("./Data/Effect/Files/ScissorsGameCloudEffect.json");

}

void TutorialActor::Update(float deltaTime)
{
    elapsedTime += deltaTime;

    // チュートリアルマネージャーの更新
    if (tutorialManager)
    {
        tutorialManager->Update(deltaTime);
    }

    UpdateShowArrowEnemy(deltaTime);

    UpdateSideArrow(deltaTime);

    UpdatePendingTutorialSpawns(deltaTime);
}

void TutorialActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat2("arrowOffsetPos", &arrowOffsetPos.x);
    //ImGui::DragFloat2("ui_text_offset", &uiTextOffsetPos.x);
#endif
}

// チュートリアル開始処理
void TutorialActor::StartTutorial()
{
    // 最初のステートに変更
    tutorialManager->ChangeState("TutorialStep_MoveStart");
}

// プレイヤーを取得する
ScissorsPlayer1* TutorialActor::GetPlayer()
{
    auto actorManager = GetOwnerScene()->GetActorManager();
    if (auto player = actorManager->GetActorOfType<ScissorsPlayer1>())
    {
        return player.get();
    }
    return nullptr;
}

// チュートリアルターゲットに登録する
void TutorialActor::AddTutorialEnemy(const std::shared_ptr<EnemyBase>& enemy, bool spawnArrowUi)
{
    if (!enemy)
        return;

    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    TutorialTargetEnemy target{};
    target.enemy = enemy;

    XMFLOAT2 imageSize = { 95.0f,77.0f };

    if (spawnArrowUi)
    {
        target.arrowImage =
            std::make_shared<UIImageComponent>(
                "./Data/Textures/ScissorsUI/Tutorial/enemy_arrow.png",
                "enemy_arrow");

        target.arrowImage->SetSize(imageSize);
        target.arrowImage->SetVisible(true);

        uiManager->Add(target.arrowImage);
    }

    tutorialTargets.push_back(target);
}

// 敵を生成する
std::shared_ptr<EnemyBase> TutorialActor::SpawnEnemy(
    const XMFLOAT3& pos,
    YarnEnemyType type, bool isBig,
    float speed, const XMFLOAT3& dir, bool isTied)
{
    auto scene = GetOwnerScene();
    if (auto tutorialScene = dynamic_cast<TutorialScene*>(scene))
    {
        enemyCount++;
        auto enemy = tutorialScene->SpawnEnemy(pos, type, isBig, speed, dir, isTied);
        enemy->onDeath = ([&]
            {
                enemyCount--;
            });
        return enemy;
    }
    Logger::Warning(U8("TutorialのSpanwEnemyでnullptrを返しています"));
    return nullptr;
}

// 縫い留められた敵がいるかどうか
bool TutorialActor::HasPinnedEnemy() const
{
    for (auto& target : tutorialTargets)
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
            continue;

        if (enemy->IsTied())
            return true;
    }

    return false;
}

// 倒された敵がいるかどうか
bool TutorialActor::HasDeadEnemy()const
{
    for (auto& target : tutorialTargets)
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
            continue;

        if (enemy->IsDead())
            return true;
    }

    return false;
}

// チュートリアルターゲットをクリアする
void TutorialActor::ClearTutorialTargets()
{
    for (auto& target : tutorialTargets)
    {
        if (target.arrowImage)
        {
            target.arrowImage->MarkPendingKill();
        }

        if (auto enemy = target.enemy.lock())
        {
            if (!enemy->IsDead())
            {
                enemy->MarkPendingKill();
            }
        }
    }

    tutorialTargets.clear();
}

// チュートリアルターゲットの矢印を非表示にする
void TutorialActor::HideTutorialTargetsArrows()
{
    for (auto& target : tutorialTargets)
    {
        if (target.arrowImage)
        {
            target.arrowImage->SetVisible(false);
        }
    }
}

// 矢印を出す
void TutorialActor::ShowArrows()
{
    // 右矢印
    arrowRightComponent->SetVisible(true);
    // 左矢印
    arrowLeftComponent->SetVisible(true);
    // 上矢印
    arrowUpComponent->SetVisible(true);
    // 下矢印
    arrowDownComponent->SetVisible(true);
}

// 矢印を消す
void TutorialActor::HideArrows()
{
    // 右矢印
    arrowRightComponent->SetVisible(false);
    // 左矢印
    arrowLeftComponent->SetVisible(false);
    // 上矢印
    arrowUpComponent->SetVisible(false);
    // 下矢印
    arrowDownComponent->SetVisible(false);
}

// 予約スポーンをする
void TutorialActor::ReserveSpawnEnemy(const XMFLOAT3& pos,
    YarnEnemyType type,
    bool isBig,
    float speed,
    const XMFLOAT3& dir,
    bool isTied, bool isTargetTutorial, bool spawnArrowUi,
    float previewTime, float spawnTime)
{
    PendingTutorialSpawn spawn{};

    spawn.position = pos;
    spawn.type = type;
    spawn.isBig = isBig;
    spawn.speed = speed;
    spawn.direction = dir;
    spawn.isTied = isTied;
    spawn.isTargetTutorial = isTargetTutorial;
    spawn.spawnArrowUi = spawnArrowUi;
    spawn.previewStartTime = previewTime;
    spawn.spawnTime = spawnTime;

    pendingTutorialSpawns.push_back(spawn);
}

// 生き残っている敵
int TutorialActor::GetAliveEnemyCount() const
{
    int count = 0;

    // 生存中
    for (auto& target : tutorialTargets)
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
            continue;

        if (enemy->IsDead())
            continue;

        count++;
    }

    // 予約中
    count += static_cast<int>(pendingTutorialSpawns.size());

    return count;
}

// 出現エフェクトを生成
void TutorialActor::SpawnPreviewEffect(DirectX::XMFLOAT3 pos)
{
    if (spawnEffectComponent)
    {
        spawnEffectComponent->SetWorldLocationDirect(pos);
        spawnEffectComponent->Play();
    }
}


// 敵の上に出す矢印
void TutorialActor::UpdateShowArrowEnemy(float deltaTime)
{
    for (auto& target : tutorialTargets)
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
        {
            if (target.arrowImage)
            {
                target.arrowImage->SetVisible(false);
            }
            continue;
        }

        // 死亡してるなら消す
        if (enemy->IsDead())
        {
            if (target.arrowImage)
            {
                target.arrowImage->SetVisible(false);
            }
            continue;
        }

        // 敵位置
        XMFLOAT3 worldPos = enemy->GetPosition();

        // 頭上
        worldPos.y += 3.0f;

        // UI変換
        XMFLOAT2 uiPos = WorldToUI(worldPos);

        // ふわふわ
        float floatOffset = sinf(elapsedTime * 4.0f) * 10.0f;

        uiPos.x += arrowOffsetPos.x;
        uiPos.y += arrowOffsetPos.y - floatOffset;

        if (target.arrowImage)
        {
            target.arrowImage->SetWorldPosition(uiPos);
            target.arrowImage->SetPivot({ 0.5f,0.5f });
            target.arrowImage->SetVisible(true);
        }
    }

}

// 壁際に出す矢印
void TutorialActor::UpdateSideArrow(float deltaTime)
{
    // ふわふわ
    float floatUpOffset = sinf(elapsedTime * 4.0f) * 7.0f;
    float floatSideOffset = sinf(elapsedTime * 4.0f) * 7.0f;

    // 右壁矢印
    DirectX::XMFLOAT3 rightWall = { 24.0f,0.0f,12.0f };
    DirectX::XMFLOAT2 rightUiPos = WorldToUI(rightWall);
    rightUiPos.x += floatSideOffset;
    arrowRightComponent->SetWorldPosition(rightUiPos);

    // 左壁矢印
    DirectX::XMFLOAT3 leftWall = { 0.0f,0.0f,12.0f };
    DirectX::XMFLOAT2 leftUiPos = WorldToUI(leftWall);
    leftUiPos.x -= floatSideOffset;
    arrowLeftComponent->SetWorldPosition(leftUiPos);

    // 上壁矢印
    DirectX::XMFLOAT3 upWall = { 12.0f,0.0f,24.0f };
    DirectX::XMFLOAT2 upUiPos = WorldToUI(upWall);
    upUiPos.y -= floatUpOffset;
    arrowUpComponent->SetWorldPosition(upUiPos);

    // 下壁矢印
    DirectX::XMFLOAT3 downWall = { 12.0f,0.0f,0.0f };
    DirectX::XMFLOAT2 downUiPos = WorldToUI(downWall);
    downUiPos.y += floatUpOffset;
    arrowDownComponent->SetWorldPosition(downUiPos);
}

// 敵の遅延湧きの更新処理
void TutorialActor::UpdatePendingTutorialSpawns(float deltaTime)
{

    for (auto& s : pendingTutorialSpawns)
    {
        s.timer += deltaTime;

        // 予告
        if (!s.previewed && s.timer >= s.previewStartTime)
        {
            SpawnPreviewEffect(s.position);

            s.previewed = true;
        }

        // 出現
        if (!s.spawned &&
            s.timer >= s.spawnTime)
        {
            auto enemy = SpawnEnemy(
                s.position,
                s.type,
                s.isBig,
                s.speed,
                s.direction,
                s.isTied);

            if (s.isTargetTutorial)
            {
                AddTutorialEnemy(enemy, s.spawnArrowUi);
            }

            s.spawned = true;
        }
    }

    pendingTutorialSpawns.erase(
        std::remove_if(
            pendingTutorialSpawns.begin(),
            pendingTutorialSpawns.end(),
            [](const auto& s)
            {
                return s.spawned;
            }),
        pendingTutorialSpawns.end());



}