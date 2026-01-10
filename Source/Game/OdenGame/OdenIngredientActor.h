#pragma once

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "OdenData/OdenDataStruct.h"
#include "OdenData/OdenShapeDataTable.h"

class OdenSlotActor;

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
        InSlot,     // 鍋の枠にある
        Dragging,   // 掴んでいる
    };

    // ドラック中にどこにマウスカーソルがあるか
    enum class EHoverTarget :uint8_t
    {
        None,
        OrderBubble,    // お題
        TrashBin,       // ゴミ箱
        OdenSlot        // スワップする枠
    };

    enum class ERotateType:uint8_t
    {
        Vertical,   // 縦回転
        Horizontal  // 横回転
    };
public:
    OdenIngredientActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // 初期の角度設定
    void SetAngleOffset(const DirectX::XMFLOAT3& angleDegree)
    {
        GetRootComponent()->SetRelativeEulerRotationDirect(angleDegree);
    }

    // 横回転するときに呼ぶ関数
    void RotateHorizontal();

    // 縦回転するときに呼ぶ関数
    void RotateVertical();

    // 回転軸を更新
    void UpdateRotation();

    // 今の食材の種類を返す
    EOdenType GetIngredientType() const { return ingredientType; }

    // 今の正面を返す topが正面になる
    EOdenFace GetCurrentFrontFace() const { return odenOrientation.top; }

    // 今見えている 形 を返す
    OdenShapeData GetCurrentShape() const
    {
        return faceShapeTable.faceShapes.at(odenOrientation.top);
    }

    // 今セットされている枠を設定する
    void SetCurrentSlot(const std::weak_ptr<OdenSlotActor>& slot)
    {
        currentSlot = slot;
    }


    // 今セットされている枠を取得する
    std::weak_ptr<OdenSlotActor> GetCurrentSlot() const
    {
        return currentSlot;
    }

private:
    // ドラック開始処理
    void TryBeginDrag(const DirectX::XMFLOAT2& cursor);

    // ドラック中の処理
    void UpdateDragging(const DirectX::XMFLOAT2& cursor) const;

    // 離した瞬間の処理
    void EndDrag(const DirectX::XMFLOAT2& cursor);

    // ドラック中のターゲットを返す
    EHoverTarget DetectHoverTarget(const DirectX::XMFLOAT2& cursor) const;

    // 離したときのターゲットのアクターを返す
    std::weak_ptr<Actor> GetHoverSlot(const DirectX::XMFLOAT2& cursor);

    // スロットで入れ替える（Drag中は呼び出さない)
    void SwapWithSlot(const std::weak_ptr<OdenSlotActor>& targetSlot);

    // 元のスロットに戻る
    void ReturnToSlot();

    // 横回転時の面の向き状態を更新
    void RotateHorizontalOrientation(OdenOrientation& o);

    // 縦回転時の面の向き状態を更新
    void RotateVerticalOrientation(OdenOrientation& o);

    // 食材を破棄した時に呼ぶ関数
    void TrashSelf();

    // 現在の上の向きに向く？
    DirectX::XMVECTOR OrientationToQuat(const OdenOrientation& o) const
    {
        return FaceToQuat(o.top);
    }

    // 食材の向きから角度を求める
    DirectX::XMVECTOR FaceToQuat(const EOdenFace face) const;

    // 回転を始める
    void StartRotationAnim(ERotateType rotateType);
protected:
    std::shared_ptr<SkeletalMeshComponent> ingredientModel; // 具材
    std::shared_ptr<BoxComponent> boxComponent; // レイキャスト判定するもの

    //DirectX::XMFLOAT3 odenIngredientAngleDegree = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 visualRotationQuat = { 0.0f,0.0f,0.0f,1.0f };

    EOdenDragState dragState = EOdenDragState::InSlot;   // おでんの状態

    EHoverTarget currentHoverTarget = EHoverTarget::None; // マウスクリック中のターゲット

    OdenOrientation odenOrientation = // 食材の向きの状態
    {
         EOdenFace::Front, EOdenFace::Back,
         EOdenFace::Left, EOdenFace::Right,
         EOdenFace::Top, EOdenFace::Bottom
    };

    EOdenType ingredientType;   // 食材の種類
    OdenFaceShapeTable faceShapeTable;  // 食材ごとの面に対応する形

    std::weak_ptr<OdenSlotActor> currentSlot; // 今の枠
    std::weak_ptr<OdenSlotActor> grabbedFromSlot; // Drag 開始時のスロット

    std::shared_ptr<EasingRunner> easingRunner; // 角度を easing させるのに使う

    DirectX::XMVECTOR startQ;
    DirectX::XMVECTOR targetQ;
    float orientationSlerpValue = 0.0f;

    
};


class OdenDaikonActor : public OdenIngredientActor
{
public:
    OdenDaikonActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};

class OdenKonnyakuActor : public OdenIngredientActor
{
public:
    OdenKonnyakuActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};
