#include "pch.h"
#include "OdenIngredient.h"

#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenBubbleActor.h"  
#include "Game/OdenGame/OdenSlotActor.h"    


void OdenIngredientActor::Update(float elapsedTime)
{
    // おでんを回転させる
    ingredientModel->SetRelativeEulerRotationDirect(odenIngredientAngleDegree);

    // 位置を取得
    DirectX::XMFLOAT3 position = GetPosition();

    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
    {
        return;
    }

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
    {
        return;
    }


    switch (dragState)
    {
    case EOdenDragState::InSlot:
    {// 鍋の中にある時

    }
    break;
    case EOdenDragState::Dragging:
        break;
    case EOdenDragState::OverOrder:
        break;
    case EOdenDragState::OverTrash:
        break;
    case EOdenDragState::Released:
        break;
    case EOdenDragState::OverSlot:
        break;
    }

    // マウスカーソルを取得
    if (InputSystem::GetInputState("MouseLeft"))
    {// 左ボタンを押している間
        DirectX::XMFLOAT2 cursor;
        if (!InputSystem::GetMousePositionUI(cursor))
        {// ビューポート外だったら、入力しない
            return;
        }

        HitResultWithActor result;
        XMFLOAT3 intersectPos = {};

        if (dragState == EOdenDragState::InSlot)
        {
            if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
            {// マウスカーソルがおでんと当たっていたら、
                intersectPos = result.hitPoint;
                dragState = EOdenDragState::Dragging;
                Logger::Log(U8("おでんとマウスカーソルがクリックされた！"));
            }
        }

        if (dragState == EOdenDragState::Dragging)
        {
            if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))  // こっちはドラッグしている時だから、判定を大きく取りたいから、
            {// マウスカーソルがおでんと当たっていたら、
                intersectPos = result.hitPoint;
                intersectPos.y = 0.0f;
                SetPosition(intersectPos);
            }
        }

        DebugDrawManager::DrawSphere(intersectPos, 0.5f, { 1, 1, 0, 1 });

        // おでんが今スクリーン上でどの位置か
        XMFLOAT2 screenPos = WorldToUI(intersectPos);
        // この position から currentHoverTarget を決定する
    }
    if (dragState == EOdenDragState::Dragging)
    {
        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {//　マウスクリックを離した瞬間
            OnMouseRelease();
            Logger::Log(U8("おでんを離した！"));
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

// マウスを離した時のターゲットを返す
OdenIngredientActor::EHoverTarget OdenIngredientActor::DetectHoverTarget(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {// マウスカーソルが
        if (auto odenSlot = dynamic_cast<OdenSlotActor*>(result.actor))
        {// おでんの枠モデルに当たっていたら、
            Logger::Log(U8("枠の上でマウスクリックを離した！"));
            return EHoverTarget::OdenSlot;
            dragState = EOdenDragState::OverSlot;
            // おでんをスワップする
            odenSlot->SwapOden(); // これどこで呼ぼうかな。。
        }
        else if (auto odenBubble = dynamic_cast<OrderBubbleActor*>(result.actor))
        {// お題の上だったら
            return EHoverTarget::OrderBubble;
        }
        //else if (auto odenBubble = dynamic_cast<OrderBubbleActor*>(result.actor))
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

    // 初期の角度調整
    //SetAngleOffset({ -70.0f,0.0f,0.0f });

    boxComponent = AddComponent<BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = ingredientModel->GetModelSize();
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::Oden);
    //boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

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
