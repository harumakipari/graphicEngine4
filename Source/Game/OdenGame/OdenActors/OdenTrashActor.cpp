#include "pch.h"
#include "OdenTrashActor.h"

void OdenTrashActor::Initialize(const Transform& transform)
{
    std::string parentName = "OrderTrash_Box";

    // “–‚½‚è”»’è‚ð“o˜^
    auto boxComponent = AddComponent<BoxComponent>(parentName);
    DirectX::XMFLOAT3 size = { 5.0f,5.0f,5.0f };
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);// ‚¨‚Å‚ñ‚ÌƒQ[ƒ€‚ÌƒJ[ƒ\ƒ‹‚Ìƒ^[ƒQƒbƒg
    boxComponent->Initialize();

}
