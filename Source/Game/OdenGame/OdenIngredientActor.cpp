#include "pch.h"
#include "OdenIngredientActor.h"

#include "OdenTrashActor.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenBubbleActor.h"  
#include "Game/OdenGame/OdenSlotActor.h"    
#include "Game/OdenGame/OdenTrashActor.h"    


void OdenIngredientActor::Update(float elapsedTime)
{
    // おでんを回転させる
    ingredientModel->SetRelativeEulerRotationDirect(odenIngredientAngleDegree);

    // 位置を取得
    DirectX::XMFLOAT3 position = GetPosition();

    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    // ① 押した瞬間：選択判定
    if (dragState == EOdenDragState::InSlot &&
        InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        TryBeginDrag(cursor);
    }

    // ② ドラッグ中：移動
    if (dragState == EOdenDragState::Dragging)
    {
        UpdateDragging(cursor);

        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {
            EndDrag(cursor);
        }
    }

}

// 具材が横回転するときに呼ぶ関数
void OdenIngredientActor::RotateHorizontal()
{
    odenIngredientAngleDegree.x += 90.0f;
    MathHelper::ClampEulerAngle(odenIngredientAngleDegree.x);

    // 内部的に回転する　右に
    RotateHorizontalOrientation(odenOrientation);
}

// 具材が縦回転するときに呼ぶ関数
void OdenIngredientActor::RotateVertical()
{
    odenIngredientAngleDegree.z += 90.0f;
    MathHelper::ClampEulerAngle(odenIngredientAngleDegree.z);

    // 内部的に回転する　奥に
    RotateVerticalOrientation(odenOrientation);
}

// マウスクリックを離した瞬間に呼ぶ関数
void OdenIngredientActor::OnMouseRelease()
{
    HitResultWithActor result;

}

void OdenIngredientActor::TryBeginDrag(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
    {
        if (result.actor == this)
        {
            dragState = EOdenDragState::Dragging;
            Logger::Log(U8("おでんを掴んだ！"));
        }
    }
}

void OdenIngredientActor::UpdateDragging(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
    {
        XMFLOAT3 pos = result.hitPoint;
        pos.y = 0.0f;
        SetPosition(pos);

        DebugDrawManager::DrawSphere(pos, 0.5f, { 1,1,0,1 });
    }
}

void OdenIngredientActor::EndDrag(const DirectX::XMFLOAT2& cursor)
{
    dragState = EOdenDragState::InSlot;
    switch (EHoverTarget s = DetectHoverTarget(cursor))
    {
    case EHoverTarget::OdenSlot:
        //HandleDropOnSlot();
        Logger::Log(U8("枠の上でマウスクリックを離した！"));
        break;
    case EHoverTarget::OrderBubble:
        Logger::Log(U8("お題の上でマウスクリックを離した！"));
        //HandleDropOnOrder();
        break;
    case EHoverTarget::TrashBin:
        Logger::Log(U8("ゴミ箱の上でマウスクリックを離した！"));
        //HandleDropOnTrash();
        break;
    default:
        //ReturnToPot();
        break;
    }

    Logger::Log(U8("おでんを離した！"));
}

// マウスを離した時のターゲットを返す
OdenIngredientActor::EHoverTarget OdenIngredientActor::DetectHoverTarget(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {// マウスカーソルが
        if (auto odenSlot = dynamic_cast<OdenSlotActor*>(result.actor))
        {// おでんの枠モデルに当たっていたら、
            return EHoverTarget::OdenSlot;
            //dragState = EOdenDragState::OverSlot;
            // おでんをスワップする
            odenSlot->SwapOden(); // これどこで呼ぼうかな。。
        }
        else if (auto odenBubble = dynamic_cast<OdenBubbleActor*>(result.actor))
        {// お題の上だったら
            return EHoverTarget::OrderBubble;
        }
        else if (auto odenBubble = dynamic_cast<OdenTrashActor*>(result.actor))
        {// ゴミ箱の上だったら
            return EHoverTarget::TrashBin;
        }
    }
    Logger::Log(U8("何もないところでマウスクリックを離した！"));
    return EHoverTarget::None;
}


// 横回転時の面の向き状態を更新
void OdenIngredientActor::RotateHorizontalOrientation(OdenOrientation& o)
{// 右に回転する
    EOdenFace oldFront = o.front;
    o.front = o.left;
    o.left = o.back;
    o.back = o.right;
    o.right = oldFront;
}

// 縦回転時の面の向き状態を更新
void OdenIngredientActor::RotateVerticalOrientation(OdenOrientation& o)
{// 奥に向かって回転する
    EOdenFace oldFront = o.front;
    o.front = o.bottom;
    o.bottom = o.back;
    o.back = o.top;
    o.top = oldFront;
}


void OdenDaikonActor::Initialize(const Transform& transform)
{
    // モデル登録
    std::string parentName = "Daikon_model";
    ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
    ingredientModel->SetModel("./Data/Models/Oden_Ingredient/Oden_Daikon.gltf");

    // 当たり判定を登録
    boxComponent = AddComponent<BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = ingredientModel->GetModelSize();
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::Oden);
    boxComponent->Initialize();

    // 食材の種類を登録
    ingredientType = EOdenType::Daikon;

    // 食材の面に対応する形を登録
    faceShapeTable = DaikonShapeTable;
}

void OdenDaikonActor::Update(float elapsedTime)
{
    OdenIngredientActor::Update(elapsedTime);
}

void OdenDaikonActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("横回転")))
    {
        RotateHorizontal();
    }

    if (ImGui::Button(U8("縦回転")))
    {
        RotateVertical();
    }

#endif
}
