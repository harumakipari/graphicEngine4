#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　フィーバーゲージのUI表示
//
class OdenUIFeverGaugeActor :public Actor
{
public:
    explicit OdenUIFeverGaugeActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

private:
    std::shared_ptr<UIImageComponent> gaugeComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<UIImageComponent> gaugeFrameComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<UIImageComponent> gaugeFrameBackComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<EasingRunner> easingRunner;

    std::shared_ptr<UIGaugeComponent> gaugeUi; // 残り時間のゲージUI

    // 何回か提出したらフィーバーモードにして、その間、食材のスコアが倍になる。
    // フィーバーモード中は、フィーバーのゲージがたまらないで、減っていく
    // フィーバーモードが終わったら、ゲージが０からまたたまっていく
    // 食材のスコアが二倍のものはキラキラのエフェクトを出して、提出した時にキラキラの状態だったかを保持しておく。
    // リザルトでキラキラの状態で提出したものはリザルトでもキラキラを表示する。
    // フィーバーモード中はUIも派手にする。  虹色とか？

    // リザルト画面で提出した具材を表示するときの動きをどうするか？→具材をどうやって動いているように表示するか、アニメーションなしで、
    // リザルト画面で提出した食材を表示していく際に、最初の二個は普通のスパンのスピードで表示するけど、そのあとはだんだん早くして最後の二個だけ、ちょっとゆっくりで表示する。
    

};
