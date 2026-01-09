#include "pch.h"
#include "OdenSlotActor.h"

#include "OdenIngredientActor.h"

void OdenSlotActor::Initialize(const Transform& transform)
{
    auto boxComponent = AddComponent<BoxComponent>("OdenSlot_BoxComponent");
    DirectX::XMFLOAT3 size = { 3.0f,2.0f,3.0f }; 
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);
    //boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    //boxComponent->SetRelativeLocationDirect({ 6.0f,0.0f,6.0f });
    boxComponent->Initialize();
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

// 食材を入れ替える
void OdenSlotActor::SwapOden()
{
    
}