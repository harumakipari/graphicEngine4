#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "OdenData/OdenDataStruct.h"
#include "OdenData/OdenShapeDataTable.h"

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

    // 今の食材の種類を返す
    EOdenType GetIngredientType() const { return ingredientType; }

    // 今の正面を返す topが正面になる
    EOdenFace GetCurrentFrontFace() const { return odenOrientation.top; }

    // 今見えている 形 を返す
    OdenShapeData GetCurrentShape() const
    {
        return faceShapeTable.faceShapes.at(odenOrientation.top);
    }


private:
    // ドラック開始処理
    void TryBeginDrag(const DirectX::XMFLOAT2& cursor);

    // ドラック中の処理
    void UpdateDragging(const DirectX::XMFLOAT2& cursor);

    // 離した瞬間の処理
    void EndDrag(const DirectX::XMFLOAT2& cursor);

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

    EOdenType ingredientType;   // 食材の種類
    OdenFaceShapeTable faceShapeTable;  // 食材ごとの面に対応する形

};


class OdenDaikonActor : public OdenIngredientActor
{
public:
    OdenDaikonActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};