#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"
#include "NumberModelDisplay.h"

// タイトル本アクター
class TitleBookActor :public BookBaseActor
{
public:
    explicit TitleBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

protected:
    // コントローラー対応の本が開く処理
    void HandlePadInput() override;


private:
    // UIの矢印ボタンを生成する
    void CreateButtonArrow();

    // マウスクリック点滅
    void UpdateMouseClickBlink(float deltaTime);

    // マウスクリックの表示を切り替える
    void ShowMouseClick(bool visible);

    // マウスクリックの点滅をリセットする
    void ResetMouseClickBlink();

private:
    NumberDisplay firstStageHighScoreDisplay;
    NumberDisplay bobbinStageHighScoreDisplay;
    NumberDisplay redirectStageHighScoreDisplay;
    NumberDisplay difficultStageHighScoreDisplay;
    NumberDisplay bossStageHighScoreDisplay;

    std::shared_ptr<SceneComponent> scoreRedirectRoot;      // 反射ステージのハイスコア
    std::shared_ptr<SceneComponent> scoreDifficultRoot;      // 難しいステージのハイスコア
    std::shared_ptr<SceneComponent> scoreBossRoot;      // ボスステージのハイスコア

    DirectX::XMFLOAT3 redirectScorePosition;
    DirectX::XMFLOAT3 difficultScorePosition;
    DirectX::XMFLOAT3 bossScorePosition;

    std::shared_ptr<UIImageComponent> controlAOnButton;   // Aボタンを表示する
    std::shared_ptr<UIImageComponent> controlAOffButton;   // Aボタンを表示する
    std::shared_ptr<UIImageComponent> mouseArrowImage;   // 矢印を表示する

    std::shared_ptr<Sprite> controlButtonOnImage; // コントローラー対応用
    std::shared_ptr<Sprite> controlButtonOffImage; // コントローラー対応用
    std::shared_ptr<Sprite> keyBoardButtonOnImage; // キーボード対応用
    std::shared_ptr<Sprite> keyBoardButtonOffImage; // キーボード対応用

    float mouseBlinkTimer = 0.0f;
    float mouseBlinkInterval = 0.6f; // 切り替え間隔
    bool isMouseClickOn = false;
    bool isUpdateMouse = true;

    float elapsedTime = 0.0f;   // 経過時間


    std::shared_ptr<UIImageComponent> rankImage; // ランク画像
    std::shared_ptr<UIImageComponent> selectImage; // セレクト画像

    // 調整値
    DirectX::XMFLOAT4 rankFirstColor = { 0.058f,0.123f,0.595f,1.0f };
    DirectX::XMFLOAT4 rankBobbinColor = {1.0f,0.733f,0.116f,1.0f};
    DirectX::XMFLOAT4 rankRedirectColor = {1.0f,0.195f,0.0f,1.0f};
    DirectX::XMFLOAT4 rankDifficultColor = { 0.01f,0.414f,0.047f,1.0f };
    DirectX::XMFLOAT4 rankBossColor = { 0.432f,0.027f,0.637f,1.0f };

    // 矢印のサイズ、位置、角度
    DirectX::XMFLOAT2 arrowSize = { 180.0f,150.0f };
    DirectX::XMFLOAT2 arrowPos = { 280.0f,758.0f };
    DirectX::XMFLOAT2 arrowBasePos = { 280.0f,758.0f };
    float arrowAngle = 32.0f;

    // Aボタンのサイズ、位置
    DirectX::XMFLOAT2 mouseSize = { 132.0f,132.0f };
    DirectX::XMFLOAT2 mousePos = { 864.0f,869.0f };

};

