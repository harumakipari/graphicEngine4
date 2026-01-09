#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "OdenData/OdenDataStruct.h"


// 具材
// （回る・掴める）
//
class OdenIngredientActor :public Actor
{
public:
    // 食材の向きの状態
    struct OdenOrientation
    {
        EOdenFace front;
        EOdenFace back;
        EOdenFace left;
        EOdenFace right;
        EOdenFace top;
        EOdenFace bottom;
    };

    // おでんの状態
    enum class EOdenDragState :uint8_t
    {
        //Idle,       // 待機　まだ並んでいない
        InSlot,     // 鍋の枠にある
        Dragging,   // 掴んでいる
        OverOrder,  // 吹き出しの上
        OverTrash,  // ゴミ箱の上
        OverSlot,   // 枠の上
        Released    // マウス離した瞬間
    };

    // ドラック中にどこにマウスカーソルがあるか
    enum class EHoverTarget
    {
        None,
        OrderBubble,    // お題
        TrashBin,       // ゴミ箱
        OdenSlot        // スワップする枠
    };

public:
    OdenIngredientActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override {}

    void Update(float elapsedTime)override;

    // 初期の角度設定
    void SetAngleOffset(const DirectX::XMFLOAT3& angleDegree)
    {
        GetRootComponent()->SetRelativeEulerRotationDirect(angleDegree);
    }

    // 横回転するときに呼ぶ関数
    void RotateHorizontal();

    // 縦回転するときに呼ぶ関数
    void RotateVertical();

private:
    // マウスクリックを離した瞬間に呼ぶ関数
    void OnMouseRelease();

    // ドラック中のターゲットを返す
    EHoverTarget DetectHoverTarget(const DirectX::XMFLOAT2& cursor);

    // 横回転時の面の向き状態を更新
    void RotateHorizontalOrientation(OdenOrientation& o);

    // 縦回転時の面の向き状態を更新
    void RotateVerticalOrientation(OdenOrientation& o);
protected:
    std::shared_ptr<SkeletalMeshComponent> ingredientModel; // 具材
    std::shared_ptr<BoxComponent> boxComponent; // レイキャスト判定するもの

    DirectX::XMFLOAT3 odenIngredientAngleDegree = { 0.0f,0.0f,0.0f };

    EOdenDragState dragState = EOdenDragState::InSlot;   // おでんの状態

    EHoverTarget currentHoverTarget = EHoverTarget::None; // マウスクリック中のターゲット

    OdenOrientation odenOrientation = // 食材の向きの状態
    {
         EOdenFace::Front, EOdenFace::Back,
         EOdenFace::Left, EOdenFace::Right,
         EOdenFace::Top, EOdenFace::Bottom
    };
};


class OdenDaikonActor : public OdenIngredientActor
{
public:
    OdenDaikonActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};