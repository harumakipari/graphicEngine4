#include "pch.h"
#include "OdenSlotActor.h"

#include "OdenIngredientActor.h"

void OdenSlotActor::Initialize(const Transform& transform)
{
    // 当たり判定を追加
    std::string parentName = "OdenSlot_BoxComponent";
    auto boxComponent = AddComponent<BoxComponent>(parentName);
    DirectX::XMFLOAT3 size = { 3.0f,2.0f,3.0f };
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);
    boxComponent->Initialize();

    // 汁のモデルを追加
    auto soupModelComponent = AddComponent<StaticMeshComponent>("Oden_Soup_Model", parentName);
    soupModelComponent->SetModel("./Data/Models/Oden_Store/Oden_SoupSurface.gltf", false);
    soupModelComponent->SetRelativeLocationDirect({ 0.0f,-0.1f,0.0f });
    soupModelComponent->overrideForwardPipelineName = "OdenSoupSurfaceMesh";
    soupModelComponent->overrideDeferredPipelineName = "OdenSoupSurfaceMesh";

    // 湯気のコンポーネントを追加
    particleComponent = this->AddComponent<class ParticleComponent>("particleComponent", parentName);
    particleComponent->Load("./Data/Effect/Files/SteamEffect.json");
    particleComponent->SetRelativeLocationDirect({ 0.0f,0.5f,0.0f });

    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
        .startDelay = 0.5f // 再生開始遅延時間
    };
    particleComponent->SetAddSettings(settings);
    particleComponent->Play();
}

void OdenSlotActor::Update(float elapsedTime)
{
    if (particleComponent)
    {
        if (!particleComponent->IsPlaying())
        {
            particleComponent->Play();
        }
    }
}

// 食材をセットする
void OdenSlotActor::SetIngredient(const std::shared_ptr<OdenIngredientActor>& newIngredient)
{
    odenIngredientActor = newIngredient;
}

// 食材を取り除く
std::shared_ptr<OdenIngredientActor> OdenSlotActor::RemoveIngredient()
{
    auto old = odenIngredientActor;
    odenIngredientActor.reset();
    return old;
}

// 食材の種類を取得する
std::shared_ptr<OdenIngredientActor> OdenSlotActor::GetIngredient() const
{
    return odenIngredientActor;
}


void OdenSlotActor::OnBeat() const
{
    auto odenActor = odenIngredientActor;
    if (!odenActor)// おでんの食材が入っていたら、
        return;

    switch (rotationType)
    {
    case ERotationType::Horizontal:
        odenActor->RotateHorizontal();
        break;
    case ERotationType::Vertical:
        odenActor->RotateVertical();
        break;
    }
}
