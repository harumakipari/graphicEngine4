#include "pch.h"

#include "ScissorsStage.h"

#include "Engine/Scene/SceneBase.h"
#include "Game/Actors/Player/Player.h"

void ScissorsStage::Initialize(const Transform& transform)
{
    std::string parentName = "ScissorsStage";

    staticMeshComponent = AddComponent<StaticMeshComponent>(parentName);
    staticMeshComponent->SetModel("./Data/TeamModels/Stage/SquareStage.glb", false, false);
    //staticMeshComponent->SetModel("./Data/TeamModels/Stage/Stage.gltf", false, false);
}

void ScissorsStage::Update(float deltaTime)
{

}