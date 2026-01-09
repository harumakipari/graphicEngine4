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


void OdenSlotActor::OnBeat() const
{
    auto odenActor = odenIngredientActor.lock();
    if (!odenActor)// ‚¨‚Å‚ñ‚ÌHŞ‚ª“ü‚Á‚Ä‚¢‚½‚çA
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

// HŞ‚ğ“ü‚ê‘Ö‚¦‚é
void OdenSlotActor::SwapOden()
{
    
}