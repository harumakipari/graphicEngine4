#include "pch.h"
#include "OdenResultSkewerActor.h"

#include "OdenActors/OdenResultIngredientActor.h"

void OdenResultSkewerActor::Initialize(const Transform& transform)
{
    // 串モデル
    poleModel = AddComponent<SkeletalMeshComponent>("odenPoleModel");
    poleModel->SetModel("./Data/Models/Oden_Result_Stage/Oden_Kushi.gltf"); // 串
    poleModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
}

void OdenResultSkewerActor::AddIngredient(const std::shared_ptr<OdenResultIngredientActor>& ingredient, int index)
{
    DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };

    if (index == 0)
    {
        pos = { 0.0f, 0.0f, 0.0f };
    }
    else
    {
        auto& prevIngredient = ingredients[index - 1];

        // 前の具材の位置
        DirectX::XMFLOAT3 prevPos = prevIngredient->GetRootComponent()->GetRelativeLocation();


        float  prevSizeY = GetIngredientYOffset(prevIngredient->GetIngredientType());
        constexpr  float space = 0.0f;

        // Y方向に「前の位置 + 高さ」
        pos.y = prevPos.y + prevSizeY + space;
    }

    ingredient->GetRootComponent()->SetRelativeLocationDirect(pos);
    ingredient->GetRootComponent()->AttachTo(GetRootComponent());

    ingredients.push_back(ingredient);
}

// 食材の種類によってオフセットを取得する
float OdenResultSkewerActor::GetIngredientYOffset(const EOdenType type)
{
    switch (type)
    {
    case EOdenType::Daikon:     return 2.0f;
    case EOdenType::Egg:        return 1.9f;
    case EOdenType::Konnyaku:   return 1.3f;
    case EOdenType::Hanpen:     return 1.3f;
    case EOdenType::Chikuwa:    return 0.8f;
    case EOdenType::Goboten:    return 0.7f;
    case EOdenType::Cake:       return 0.9f;
    case EOdenType::Shirataki:  return 0.8f;
    case EOdenType::Kobumusubi:  return 0.8f;
    case EOdenType::Tsukune:    return 1.3f;
    case EOdenType::Donut:      return 2.0f;
    default:                              return 1.5f;
    }
}