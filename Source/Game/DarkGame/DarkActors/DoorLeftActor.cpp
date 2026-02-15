#include "pch.h"
#include "DoorLeftActor.h"

void DoorLeftActor::Initialize(const Transform& transform)
{
    std::string parentName = "Door_Left";

    meshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    meshComponent->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
}

void DoorLeftActor::Update(float deltaTime)
{

}

// ƒvƒŒƒCƒ„[‚ª‰Ÿ‚µ‚½‚ÉŒÄ‚Ô 
void DoorLeftActor::Interact(Player* player)
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}
