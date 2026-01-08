#include "pch.h"
#include "OdenIngredient.h"

#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"


// 具材が横回転するときに呼ぶ関数
void OdenIngredientActor::RotateHorizontal()
{
    odenIngredientAngleDegree.x += 90.0f;
    MathHelper::ClampEulerAngle(odenIngredientAngleDegree.x);
}

// 具材が縦回転するときに呼ぶ関数
void OdenIngredientActor::RotateVertical()
{
    odenIngredientAngleDegree.z += 90.0f;
    MathHelper::ClampEulerAngle(odenIngredientAngleDegree.z);
}

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
        if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
        {// マウスカーソルがおでんと当たっていたら、
            intersectPos = result.hitPoint;
            Logger::Log(U8("おでんとマウスカーソルが当たった！"));
        }
        DebugDrawManager::DrawSphere(intersectPos, 0.5f, { 1, 1, 0, 1 });
    }
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
