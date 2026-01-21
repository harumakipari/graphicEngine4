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

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // フォントをセットする
    void SetFontAndMakeTextComponent();

private:
    // 演出のためにスコアを加算する
    void AddScore(int add);

private:
    std::shared_ptr<UITextComponent> scoreTextUi; // スコアのテキスト描画
    std::shared_ptr<UITextComponent> scoreBackTextUi; // スコアのテキスト描画
    std::vector<std::shared_ptr<OdenResultIngredientActor>> resultIngredients; // 具材
    std::shared_ptr<EasingRunner> easingRunner; // イージングランナー

    float nextSpawnDelay = 0.5f; // 次の具材が出現するまでの遅延時間
    float spawnTimer = 0.0f; // 出現タイマー
    size_t spawnIndex = 0; // 出現インデックス

    int displayScore = 0;   // 表示中のスコア
    float popupOffsetY = 0.0f;
    XMFLOAT2 baseScorePos = {}; //　スコア表示位置
};
