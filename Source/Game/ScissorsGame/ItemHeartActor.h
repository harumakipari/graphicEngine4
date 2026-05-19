#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"


class ItemHeartActor :public Actor
{
    enum class ItemState :uint8_t
    {
        Preparing,  // 準備中
        Falling,    // 落下中
        Waiting,    // 地面で待機
        Used,    // 使用済み
        Hide
    };

public:
    explicit ItemHeartActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // ボス位置から目的位置まで発射する処理
    void LaunchTo(const DirectX::XMFLOAT3& targetPos);

    // アイテムの見た目を非表示にする
    void HideItemVisual();

protected:
    // アイテムを使用する
    void UseItem(); // 継承してこの関数を使用できるように後程変更

    // アイテム待機中の動き
    void UpdateWaiting(float deltaTime); // アイテムのステージ上にあるの動き　ハートなら揺れる、飴なら回転など。

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加

    ItemState itemState = ItemState::Preparing;
    float elapsedTime;    // 経過時間
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };
    float gravity = 4.9f;
    float groundY = 0.5f;   // 地面の基準点

    DirectX::XMFLOAT3 basePosition = { 0.0f,0.5f,0.0f };
};

