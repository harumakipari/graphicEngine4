#include "pch.h"
#include "TutorialActor.h"
#include "TutorialStep.h"
#include "Physics/CollisionFunction.h"
#include "TutorialManager.h"
#include "ScissorsPlayer1.h"
#include "TutorialScene.h"
#include "EnemyBase.h"

void TutorialActor::Initialize(const Transform& transform)
{
    tutorialManager = std::make_unique<TutorialManager>();
    // 各ステートを登録
    tutorialManager->RegisterState(std::make_unique<TutorialStep_MoveStart>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ChargeStart>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SpawnStaticEnemy>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_TiedEnemy>(this));
}

void TutorialActor::Update(float deltaTime)
{
    // チュートリアルマネージャーの更新
    if (tutorialManager)
    {
        tutorialManager->Update(deltaTime);
    }

    UpdateShowArrowEnemy();
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
void TutorialActor::AddTutorialTarget(const std::shared_ptr<EnemyBase>& enemy)
{
    if (!enemy)
        return;

    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    TutorialTarget target{};
    target.enemy = enemy;

    XMFLOAT2 imageSize = { 95.0f,77.0f };

    target.arrowImage =
        std::make_shared<UIImageComponent>(
            "./Data/Textures/ScissorsUI/Tutorial/enemy_arrow.png",
            "enemy_arrow");

    target.arrowImage->SetSize(imageSize);
    target.arrowImage->SetVisible(true);

    uiManager->Add(target.arrowImage);

    tutorialTargets.push_back(target);
}

// 敵を生成する
std::shared_ptr<EnemyBase> TutorialActor::SpawnEnemy(
    const XMFLOAT3& pos,
    YarnEnemyType type, bool isBig,
    float speed, const XMFLOAT3& dir)
{
    auto scene = GetOwnerScene();
    if (auto tutorialScene = dynamic_cast<TutorialScene*>(scene))
    {
        return tutorialScene->SpawnEnemy(pos, type, isBig, speed, dir);
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

// 敵の上に出す矢印
void TutorialActor::UpdateShowArrowEnemy()
{
    for (auto& target : tutorialTargets)
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
        {
            target.arrowImage->SetVisible(false);
            continue;
        }

        // 死亡してるなら消す
        if (enemy->IsDead())
        {
            target.arrowImage->SetVisible(false);
            continue;
        }

        // 敵位置
        XMFLOAT3 worldPos = enemy->GetPosition();

        // 頭上
        worldPos.y += 3.0f;

        // UI変換
        XMFLOAT2 uiPos = WorldToUI(worldPos);

        uiPos.x += arrowOffsetPos.x;
        uiPos.y += arrowOffsetPos.y;

        target.arrowImage->SetWorldPosition(uiPos);
        target.arrowImage->SetPivot({ 0.5f,0.5f });
        target.arrowImage->SetVisible(true);
    }

}
