#pragma once
#include "Core/Actor.h"
#include "OdenActors/OdenResultIngredientActor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　リザルトスコアのUI表示
//
class OdenResultScoreActor :public Actor
{
public:
    explicit OdenResultScoreActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override {}

    // フォントをセットする
    void SetFontAndMakeTextComponent();

private:
    // 演出のためにスコアを加算する
    void AddScore(int add);

    // spawnIndexに応じてスポーンタイムを変更する
    float CalcSpawnDelay() const;

private:
    std::shared_ptr<UITextComponent> scoreTextUi; // スコアのテキスト描画
    std::vector<std::shared_ptr<OdenResultIngredientActor>> resultIngredients; // 具材
    std::shared_ptr<EasingRunner> easingRunner; // イージングランナー

    std::shared_ptr<UIImageComponent> noMissOrderUi; // 注文ミスをしないUI描画
    std::shared_ptr<UIImageComponent> totalCountUi; // 合計個数UI描画
    std::shared_ptr<UIImageComponent> streakCountUi; // 連続正解の個数UI描画
    std::shared_ptr<UIImageComponent> feverCountUi; // フィーバー中の提供の個数UI描画

    std::shared_ptr<UITextComponent> noMissScoreTextUi; // 注文ミスをしないスコア数字描画
    std::shared_ptr<UITextComponent> totalCountTextUi; // 合計個数の数字描画
    std::shared_ptr<UITextComponent> streakCountTextUi; // 連続正解の個数テキスト描画
    std::shared_ptr<UITextComponent> feverCountTextUi; // フィーバー中の提供の個数テキスト描画

    std::shared_ptr<UIImageComponent> rankSUi; // ランクS
    std::shared_ptr<UIImageComponent> rankAUi; // ランクA
    std::shared_ptr<UIImageComponent> rankBUi; // ランクB
    std::shared_ptr<UIImageComponent> rankCUi; // ランクC
    std::shared_ptr<UIImageComponent> rankDUi; // ランクD

    std::shared_ptr<UIImageComponent> nextRankSUi; // 次のランクSの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankAUi; // 次のランクAの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankBUi; // 次のランクBの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankCUi; // 次のランクCの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankDUi; // 次のランクDの時に表示するUI

    float nextSpawnDelay = 0.5f; // 次の具材が出現するまでの遅延時間
    float spawnTimer = 0.0f; // 出現タイマー
    size_t spawnIndex = 0; // 出現インデックス

    int displayScore = 0;   // 表示中のスコア
    float popupOffsetY = 0.0f;
    XMFLOAT2 baseScorePos = {}; //　スコア表示位置
};
