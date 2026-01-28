#include "pch.h"
#include "OdenResultSkewerActor.h"

#include "OdenActors/OdenResultIngredientActor.h"

void OdenResultSkewerActor::Initialize(const Transform& transform)
{
    // 串モデル
    poleModel = AddComponent<SkeletalMeshComponent>("odenPoleModel");
    poleModel->SetModel("./Data/Models/Oden_Result_Stage/Oden_Kushi.gltf"); // 串
    poleModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    easingRunner = AddComponent<CoreEasingComponent>("easingComponent", "odenPoleModel");
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


// 回転開始
void OdenResultSkewerActor::StartRotateOneTurn()
{
    using namespace DirectX;

    XMVECTOR startQ = XMLoadFloat4(
        &GetRootComponent()->GetRelativeRotation()
    );

    constexpr float totalAngle = 720.0f; // 720°

    TestEasingHandler handler;
    handler.AddEasing(TestEaseType::OutCubic, 0.0f, 1.0f, 2.5f);

    handler.SetCompletedFunction([this, startQ]()
        {
            if (onRotationFinished)
                onRotationFinished();
        });

    PropertyAccessor<float> accessor;
    accessor.getter = []() { return 1.0f; };
    accessor.setter = [this, startQ](float t)
        {
#if 0
            float angle = totalAngle * t;

            XMVECTOR delta =
                XMQuaternionRotationAxis(
                    XMVectorSet(0, 1, 0, 0),
                    angle
                );

            XMVECTOR q = XMQuaternionMultiply(startQ, delta);
            q = XMQuaternionNormalize(q);

            DirectX::XMFLOAT4 out;
            XMStoreFloat4(&out, q);
            GetRootComponent()->SetRelativeRotationDirect(out);
#else
            XMFLOAT3 angleDegree = GetRootComponent()->GetRelativeEulerRotation();
            angleDegree.y = totalAngle * t;
            GetRootComponent()->SetRelativeEulerRotationDirect(angleDegree);
#endif // 0
        };

    easingRunner->StartHandler(handler, accessor);
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